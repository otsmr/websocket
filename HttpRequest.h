/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include "Url.h"

namespace HTTP {
    
class HttpRequest {
public:

    enum Method {
        Invalid,
        GET
    };
    
    struct Header {
        std::string name;
        std::string value;
    };

    HttpRequest();
    ~HttpRequest();

    std::vector<Header> headers() { return m_headers; };
    Header get_header(const std::string& name);

    Method method() const { return m_method; }
    Url url() const { return m_url; }

    HttpRequest from_raw_request(std::vector<uint8_t> raw_request);

private:

    Method m_method { Method::Invalid };
    std::vector<Header> m_headers;
    std::vector<uint8_t> m_body;
    Url m_url;
    
};

} // namespace HTTP