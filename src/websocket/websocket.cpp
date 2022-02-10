/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include "websocket.h"

WebSocket::WebSocket(int connection)
{
    m_connection = connection;
}

void WebSocket::send_message(std::string message) {

    std::vector<uint8_t> raw_frame = DataFrame::get_text_frame(message).get_raw_frame();
    send(m_connection, raw_frame.data(), raw_frame.size(), 0);

}

void WebSocket::handle_frame(DataFrame frame)
{

    std::string message;
    std::string msg;
    DataFrame pong_frame;
    std::vector<uint8_t> raw_frame;

    const auto p1 = std::chrono::system_clock::now();
    std::time_t today_time = std::chrono::system_clock::to_time_t(p1);

    switch (frame.m_opcode)
    {

    case DataFrame::ConectionClose:
        if (frame.m_payload_len_bytes >= 2) {
            m_close_statuscode = frame.m_application_data.at(0) << 8;
            m_close_statuscode += frame.m_application_data.at(1) & 0xff;
        }
        close(true);
        break;

    case DataFrame::Pong:
        m_waiting_for_pong = false;
        break;

    case DataFrame::Ping:

        std::cout << "Ping from the Browser: " << std::ctime(&today_time) << std::endl;

        pong_frame.m_fin = true;
        pong_frame.m_mask = false;
        pong_frame.m_rsv = 0;
        pong_frame.m_opcode = DataFrame::Pong;
        pong_frame.m_payload_len_bytes = 0;

        raw_frame = pong_frame.get_raw_frame();
        send(m_connection, raw_frame.data(), raw_frame.size(), 0);

        break;

    // case DataFrame::BinaryFrame:
    case DataFrame::TextFrame:

        m_framequeue.push_back(frame);

        if (!frame.m_fin)
            return;

        for (DataFrame& f : m_framequeue)
            message += f.get_utf8_string();

        if (m_on_message != nullptr)
            m_on_message(message);

#if 0
        for (size_t i = 0; i < ((message.size() > 50) ? 10 : message.size()); i++)
            msg += message.at(i);

        if (message.size() > 50) {
            msg += "...";
            for (size_t i = message.size()-10; i < message.size()-1; i++)
                msg += message.at(i);
        }

        std::cout << "[WebSocket " << m_connection << "] Message: " << msg << "\n";
#endif

        m_framequeue.clear();

        break;
    
    default:
        std::cout << "frame.opcode NOT IMPLEMEMTED: (" << frame.m_opcode << ")  \n";
        break;
    }


}

void WebSocket::check_for_keep_alive() {

    // check if the client is alive
    std::thread ([&]() {

        std::vector<uint8_t> raw_frame;

        while (m_state > State::WaitingForHandshake)
        {

            std::this_thread::sleep_for(std::chrono::seconds(20));

            raw_frame = DataFrame::get_ping_frame().get_raw_frame();
            send(m_connection, raw_frame.data(), raw_frame.size(), 0);

            m_waiting_for_pong = true;

            std::this_thread::sleep_for(std::chrono::seconds(m_close_timeout));

            if (m_waiting_for_pong) {
                std::cout << "[WebSocket " << m_connection << "] no pong\n";
                m_close_statuscode = 1002;
                close(0);
                break;
            }

        }
    }).detach();

}

void WebSocket::listen()
{

    m_state = State::WaitingForHandshake;

    check_for_keep_alive();

    uint8_t buffer[MAX_PACKET_SIZE];
    DataFrame last_frame;

    size_t offset;
    size_t bytes_read;

    while (bytes_read = read(m_connection, buffer, MAX_PACKET_SIZE) > 0) {

        if (m_state < State::WaitingForHandshake)
            break;

        if (m_state == State::WaitingForHandshake) {
            
            if (!handshake(buffer)) {
                close(true);
                return;
            }

            const auto p1 = std::chrono::system_clock::now();
            std::time_t today_time = std::chrono::system_clock::to_time_t(p1);
            std::cout << "Connected: " << std::ctime(&today_time) << std::endl;

            m_state = State::Connected;
            continue;

        }

        offset = 0;

        if (m_state == State::InDataPayload) {

            offset = last_frame.add_payload_data(buffer, 0, bytes_read);

            if (!last_frame.payload_full())
                continue;

            m_state = State::Connected;
            handle_frame(last_frame);

            if (offset == bytes_read)
                continue; // No more data available
    
        }

        DataFrame frame;

        while (offset < bytes_read) {

            DataFrame current_frame;
            offset += current_frame.parse_raw_frame(buffer+offset, bytes_read-offset);
            frame = current_frame;

            if (!frame.payload_full())
                break;

        }
        
        if (frame.payload_full() == 0) {
            last_frame = frame;
            m_state = State::InDataPayload;
            continue;
        }

        handle_frame(frame);
        
    }
    
}

bool WebSocket::handshake(uint8_t buffer[MAX_PACKET_SIZE]) {

    std::vector<uint8_t> raw_data(buffer, buffer+MAX_PACKET_SIZE);

    HTTP::Request request;
    request.init_from_raw_request(raw_data);

    /* The value of this header field MUST be a
     * nonce consisting of a randomly selected 16-byte value that has
     * been base64-encoded.
     */
    char sec_key[24+37]{};
    strncpy(sec_key, request.get_header("sec-websocket-key").value.c_str(), 24);
    strncpy(sec_key+24, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11\00", 37);

    uint8_t sha1_hash[20];
    sha1((uint8_t *) sec_key, sha1_hash, 24+36);

    char b64_output[29]{};
    Base64::encode(sha1_hash, b64_output, 20);

    auto extensions = request.header_value_as_array("sec-websocket-extensions");

    for (auto & extension : extensions) {   
        if (extension == "permessage-deflate") {
            m_extensions |= PermessageDeflate;
        }
    }

    HTTP::Response response;

    response.set_header("Upgrade", "websocket");
    response.set_header("Connection", "Upgrade");
    response.set_header("Sec-WebSocket-Accept", b64_output);
    // response.set_header("Sec-WebSocket-Protocol", "");
    response.set_header("Sec-WebSocket-Version", "13");

    std::vector<uint8_t> raw = response.get_raw_response();

    send(m_connection, raw.data(), raw.size(), 0);

    return true;

}

void WebSocket::close(bool close_frame_received) {

    if (m_state == State::Disconnected)
        return;

    if (m_state != State::Closing) {

        m_state = State::Closing;

        DataFrame frame;

        frame.m_fin = true;
        frame.m_mask = false;
        frame.m_rsv = 0;
        frame.m_opcode = DataFrame::ConectionClose;
        frame.m_payload_len_bytes = 2;

        uint16_t statuscode = m_close_statuscode;
        if (close_frame_received)
            statuscode = 1000;

        frame.m_application_data.push_back((statuscode >> 8));
        frame.m_application_data.push_back((statuscode & 0xff));

        std::vector<uint8_t> raw_res = frame.get_raw_frame();
        send(m_connection, raw_res.data(), raw_res.size(), 0);

    }

    if (!close_frame_received) { 

        for (size_t i = 0; i <= m_close_timeout; i++)
        {
            if (i == m_close_timeout)
                break;
            if (m_state == State::Disconnected)
                return; // thread -> close_frame_received = 1

            sleep(1);
        }

        std::cout << "Closing timeout\n";

    }

    // Close WebSocket  ...
    std::cout << "[WebSocket " << m_connection << "] closed ("<<m_close_statuscode << ")\n";
    ::close(m_connection);
    m_state = State::Disconnected;

}