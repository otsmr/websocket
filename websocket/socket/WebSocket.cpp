/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include "WebSocket.h"

#include<iostream>
#include<string>
#include <stdio.h>
#include<fstream>

#define CLEAR_BUFFER for (size_t i = 0; i < MAX_PACKET_SIZE; i++) buffer[i] = '\00';

WebSocket::WebSocket(int connection)
{
    m_connection = connection;
}
WebSocket::~WebSocket() {

}

void WebSocket::handle_frame(DataFrame frame)
{

    if (frame.m_opcode == DataFrame::Opcode::ConectionClose) {
        std::cout << "Client want to close\n";
        return;
    }

    if (frame.m_opcode > DataFrame::Opcode::BinaryFrame) {
        std::cout << "frame.opcode NOT IMPLEMEMTED: (" << frame.m_opcode << ")  \n";
        return;
    }

    m_framequeue.push_back(frame);

    if (frame.m_fin == DataFrame::Bool::NotSet)
        return;

    std::string message;

    std::cout << "[WebSocket " << m_connection << "] Message ( ";

    for (DataFrame& frame : m_framequeue) {
        std::cout << frame.m_payload_len_bytes << " ";
        message += frame.get_utf8_string();
    }

    std::string msg = "";
    for (size_t i = 0; i < ((message.size() > 10) ? 10 : message.size()); i++)
        msg += message.at(i);

    if (message.size() > 10) {
        msg += "...";
        for (size_t i = message.size()-10; i < message.size()-1; i++)
            msg += message.at(i);
    }

    std::cout << ") " << msg << "\n";

    m_framequeue.clear();    

}

void WebSocket::listen()
{

    std::cout << "[WebSocket " << m_connection << "] open\n";

    m_state = State::WaitingForHandshake;

    uint8_t buffer[MAX_PACKET_SIZE];
    DataFrame last_frame;

    int offset;

    std::fstream f;
    f.open("buffer.txt", std::ofstream::out | std::ofstream::trunc);
    f.close();

    CLEAR_BUFFER;

    ssize_t bytes_read;

    while (
        (bytes_read = read(m_connection, buffer, MAX_PACKET_SIZE)) > 0 &&
        m_state >= State::WaitingForHandshake) {

        if (m_state == State::WaitingForHandshake) {
            
            if (handshake(buffer) < 0) {
                close();
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

        // f << "\nXXXXXX -> offset = " << offset;
        // f << "\n----------------------------------------------------- \n";
        
        // f.close();

        DataFrame frame;

        while (offset < bytes_read) {

// 818af29d c26982f4 ac0ed2ed 01df9cfa  [ 0x00 * 4080 ]
// 818ae41a f7199473 997ec46a 34af8a7d 818ada67 1837aa0e 7650fa17 db81b4 [ 0x00 * 4065 ]
// 818ab0d9 cebac0b0 a0dd90a9 0d0cdebe  [ 0x00 * 4080 ]

            DataFrame current_frame;
            offset += current_frame.parse_raw_frame(buffer+offset, (int) bytes_read-offset);
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

    std::cout << "[WebSocket " << m_connection << "] closed by the client.\n";
    close();

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