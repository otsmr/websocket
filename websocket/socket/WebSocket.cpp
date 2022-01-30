/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include "WebSocket.h"

WebSocket::WebSocket(int connection) {
    m_connection = connection;
}

void WebSocket::listen() {

    std::cout << "[WebSocket " << m_connection << "] open\n";

    m_state = State::WaitingForHandshake;

    uint8_t buffer[MAX_PACKET_SIZE];

    while (
        auto bytesRead = read(m_connection, buffer, MAX_PACKET_SIZE) > 0 &&
        m_state >= State::WaitingForHandshake) {

        if (m_state == State::WaitingForHandshake) {
            
            if (handshake(buffer) < 0) {
                close();
                return;
            }
            m_state = State::Connected;
            continue;

        }

        std::cout << "[WebSocket " << m_connection << "] Dataframe: " << buffer;

        send(m_connection, buffer, 255, 0);
    }

    std::cout << "[WebSocket " << m_connection << "] closed by the client.\n";
    close();

}

int WebSocket::handshake(uint8_t buffer[MAX_PACKET_SIZE]) {

    std::cout << buffer;

    std::vector<uint8_t> raw_data(buffer, buffer+MAX_PACKET_SIZE);

    HTTP::HttpRequest request;
    request.from_raw_request(raw_data);

    std::string sec_key = 
        request.get_header("sec-websocket-key").value +
        "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    uint8_t hash[20];
    sha1((uint8_t *) sec_key.c_str(), hash, sec_key.size());

    char * output = NULL;
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

    uint8_t raw_response[MAX_PACKET_SIZE];

    std::copy(raw.begin(), raw.end(), raw_response);

    send(m_connection, raw_response, raw.size(), 0);

    return 1;

}

void WebSocket::close() {

    if (State::Disconnected)
        return;

    m_state = State::Closing;
    // Close WebSocket  ...
    std::cout << "[WebSocket " << m_connection << "] closed\n";
    ::close(m_connection);
    m_state = State::Disconnected;

}