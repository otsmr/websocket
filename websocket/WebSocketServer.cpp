/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include <iostream>
#include <string>
#include "base64/base64.h"
#include "sha1/sha1.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"

int main () {

    std::string header = 
        "GET /chat HTTP/1.1\n" \
        "Host: server.example.com\n" \
        "Upgrade: websocket\n" \
        "Connection: Upgrade\n" \
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\n" \
        "Origin: http://example.com\n" \
        "Sec-WebSocket-Protocol: chat, superchat\n" \
        "Sec-WebSocket-Version: 13";

    HTTP::HttpRequest request;

    std::vector<uint8_t> raw_header(header.begin(), header.end());
    raw_header.assign(header.begin(), header.end());

    request.from_raw_request(raw_header);

    // std::vector<HTTP::HttpRequest::Header> headers = request.headers();
    // for (const HTTP::HttpRequest::Header& header : headers) 
    //     std::cout << "\n" << header.name << ": " << header.value;

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
        "Sec-WebSocket-Protocol", "chat"
    };
    for (size_t i = 0; i < res_headers.size(); i+=2)
        response.set_header(res_headers[i], res_headers[i+1]);

    std::cout << "\n\n --- IST ---- \n";
    std::vector<uint8_t> raw = response.get_raw_response();
    for (size_t i = 0; i < raw.size(); i++)
        std::cout << raw.at(i);

    std::cout <<
        "\n\n --- SOLL ---- \n" \
        "HTTP/1.1 101 Switching Protocols\n" \
        "Upgrade: websocket\n" \
        "Connection: Upgrade\n" \
        "Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\n" \
        "Sec-WebSocket-Protocol: chat\n";

    return 0;

}