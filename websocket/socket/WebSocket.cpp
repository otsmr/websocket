/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include "WebSocket.h"

WebSocket::WebSocket(int connection)
{
    m_connection = connection;
}
WebSocket::~WebSocket() = default;

void WebSocket::handle_frame(DataFrame frame)
{

    std::string message;
    std::string msg;
    DataFrame pong_frame;
    std::vector<uint8_t> raw_frame;

    switch (frame.m_opcode)
    {

    case DataFrame::ConectionClose:
        if (frame.m_payload_len_bytes >= 2) {
            m_close_statuscode = frame.m_application_data.at(0) << 8;
            m_close_statuscode += frame.m_application_data.at(1) & 0xff;
        }
        close(1);
        break;

    case DataFrame::Pong:
        m_waiting_for_pong = 0;
        break;

    case DataFrame::Ping:

        pong_frame.m_fin = DataFrame::Set;
        pong_frame.m_mask = DataFrame::NotSet;
        pong_frame.m_rsv = 0;
        pong_frame.m_opcode = DataFrame::Pong;
        pong_frame.m_payload_len_bytes = 0;

        raw_frame = pong_frame.get_raw_frame();
        send(m_connection, raw_frame.data(), raw_frame.size(), 0);

        break;

    // case DataFrame::BinaryFrame:
    case DataFrame::TextFrame:

        m_framequeue.push_back(frame);

        if (frame.m_fin == DataFrame::Bool::NotSet)
            return;

        std::cout << "[WebSocket " << m_connection << "] Message: ";

        for (DataFrame& f : m_framequeue)
            message += f.get_utf8_string();

        for (size_t i = 0; i < ((message.size() > 50) ? 10 : message.size()); i++)
            msg += message.at(i);

        if (message.size() > 50) {
            msg += "...";
            for (size_t i = message.size()-10; i < message.size()-1; i++)
                msg += message.at(i);
        }

        std::cout << msg << "\n";

        m_framequeue.clear();

        break;
    
    default:
        std::cout << "frame.opcode NOT IMPLEMEMTED: (" << frame.m_opcode << ")  \n";
        break;
    }


}

void WebSocket::listen_from_client() {

    uint8_t buffer[MAX_PACKET_SIZE];
    DataFrame last_frame;

    size_t offset;

    // clean file
    // std::fstream f;
    // f.open("buffer.txt", std::ofstream::out | std::ofstream::trunc);
    // f.close();

    size_t bytes_read;

    while (
        (bytes_read = read(m_connection, buffer, MAX_PACKET_SIZE)) > 0 &&
        m_state >= State::WaitingForHandshake) {

        if (m_state == State::WaitingForHandshake) {
            
            if (handshake(buffer) < 0) {
                close(1);
                return;
            }

            m_state = State::Connected;
            continue;

        }

        // std::fstream f;
        // f.open("buffer.txt", std::ios::app);
        // int j;
        // for (size_t i = 0; i < bytes_read; i++)
        // {
        //     if (i < (bytes_read-17)) {
        //         for (j = i; j < (i+17); j++) {
        //             if (buffer[j] != '\00')
        //                 break;
        //         }
        //         if (j > i+15) {
        //             f << " [ 0x00 * " << (bytes_read) - i << " ]\n";
        //             break;
        //         }
        //     }
        //     char hex[4];
        //     snprintf(hex, 4, "%02x", buffer[i]);
        //     f << hex[0] << hex[1];
        //     if ((i+1) % 4 == 0)
        //         f << " ";
        //     if ((i+1) % (4*8) == 0)
        //         f << "\n";
        // }
        // f << "\n----------------------------------------------------- \n";
        // f.close();

        offset = 0;

        if (m_state == State::InDataPayload) {

            offset = last_frame.add_payload_data(buffer, 0, bytes_read);

            if (last_frame.payload_full() == 0) {
                continue;
            }

            if (last_frame.payload_full() == 1) {
                m_state = State::Connected;
                handle_frame(last_frame);
            }

            if (offset == bytes_read) {
                continue;
            }
    
        }

        DataFrame frame;

        while (offset < bytes_read) {

            // 818af29d c26982f4 ac0ed2ed 01df9cfa  [ 0x00 * 4080 ]
            // 818ae41a f7199473 997ec46a 34af8a7d 818ada67 1837aa0e 7650fa17 db81b4 [ 0x00 * 4065 ]
            // 818ab0d9 cebac0b0 a0dd90a9 0d0cdebe  [ 0x00 * 4080 ]

            DataFrame current_frame;
            offset += current_frame.parse_raw_frame(buffer+offset, bytes_read-offset);
            frame = current_frame;

            if (frame.payload_full() == 0)
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

void WebSocket::listen()
{

    std::cout << "[WebSocket " << m_connection << "] open\n";

    m_state = State::WaitingForHandshake;

    std::thread ([&]() {
        listen_from_client();
    }).detach();

    // std::vector<uint8_t> raw_frame;

    // while (m_state >= State::WaitingForHandshake)
    // {

    //     raw_frame = DataFrame::get_ping_frame().get_raw_frame();
    //     send(m_connection, raw_frame.data(), raw_frame.size(), 0);

    //     m_waiting_for_pong = 1;

    //     sleep(m_close_timeout);

    //     if (m_waiting_for_pong == 1) {
    //         std::cout << "[WebSocket " << m_connection << "] no pong\n";
    //         m_close_statuscode = 1002;
    //         close(0);
    //         break;
    //     }

    //     // if (m_state > State::WaitingForHandshake) {

    //     //     std::string text = "ping " + std::to_string(counter);

    //     //     if (counter == 0) {
    //     //         text = "pong";
    //     //     }

    //     //     raw_frame = DataFrame::get_text_frame(text).get_raw_frame();
    //     //     send(m_connection, raw_frame.data(), raw_frame.size(), 0);

    //     // }

    //     sleep(10);

    // }
    
}

int WebSocket::handshake(uint8_t buffer[MAX_PACKET_SIZE]) {

    std::vector<uint8_t> raw_data(buffer, buffer+MAX_PACKET_SIZE);

    HTTP::HttpRequest request;
    request.from_raw_request(raw_data);

    std::string sec_key = 
        request.get_header("sec-websocket-key").value +
        "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    uint8_t hash[20];
    sha1((uint8_t *) sec_key.c_str(), hash, sec_key.size());

    char * output = nullptr;
    base64_encode(hash, &output, 20);

    HTTP::HttpResponse response;

    std::vector<std::string> res_headers= {
        "Upgrade", "websocket",
        "Connection", "Upgrade",
        "Sec-WebSocket-Accept", output,
        // "Sec-WebSocket-Protocol", "chat"
        "Sec-WebSocket-Version", "13"
    };

    for (size_t i = 0; i < res_headers.size(); i+=2)
        response.set_header(res_headers[i], res_headers[i+1]);

    std::vector<uint8_t> raw = response.get_raw_response();

    send(m_connection, raw.data(), raw.size(), 0);

    return 1;

}

void WebSocket::close(int close_frame_received) {

    if (m_state == State::Disconnected)
        return;

    if (m_state != State::Closing) {

        m_state = State::Closing;

        DataFrame frame;

        frame.m_fin = DataFrame::Set;
        frame.m_mask = DataFrame::NotSet;
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

    if (close_frame_received == 0) { 

        for (size_t i = 0; i <= m_close_timeout; i++)
        {
            if (i == m_close_timeout)
                break;
            if (m_state == State::Disconnected)
                return; // thread -> close_frame_received = 1

            sleep(1000);
        }

        std::cout << "Closing timeout\n";

    }

    // Close WebSocket  ...
    std::cout << "[WebSocket " << m_connection << "] closed ("<<m_close_statuscode << ")\n";
    ::close(m_connection);
    m_state = State::Disconnected;

}