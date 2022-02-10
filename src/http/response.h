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
    
class Response {
public:

    Response() = default;
    ~Response() = default;

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

    // const char m_protocol[9] = "HTTP/1.1";
    Statuscode m_statuscode { SwitchingProtocols };
    std::vector<Header> m_headers;
    
};

} // namespace HTTP