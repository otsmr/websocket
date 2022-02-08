/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <utility>

#define linebreak "\r\n"

namespace HTTP {
    
class HttpResponse {
public:

    HttpResponse() = default;
    ~HttpResponse() = default;

    enum Statuscode {
        SwitchingProtocols = 101,
        BadRequest = 400
    };
    
    struct Header {
        std::string name;
        std::string value;
    };

    Header set_header(std::string name, std::string value);
    std::vector<uint8_t> get_raw_response();

private:

    std::string m_protocol = "HTTP/1.1";
    Statuscode m_statuscode { Statuscode::SwitchingProtocols };
    std::vector<Header> m_headers;
    std::vector<uint8_t> m_body;
    
};

} // namespace HTTP