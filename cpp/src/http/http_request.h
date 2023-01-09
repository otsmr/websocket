/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

namespace HTTP {
    
class Request {
public:

    Request() = default;
    ~Request() = default;

    enum Method {
        Invalid,
        GET
    };
    
    struct Header {
        std::string name;
        std::string value;
    };

    struct Url {
        std::string host;
        std::string path;
        std::string query;
    };

    Method method() const { return m_method; }
    Url url() const { return m_url; }
    std::vector<Header> headers() { return m_headers; };

    Header get_header(const std::string& name);
    std::vector<std::string> header_value_as_array(std::string name);

    size_t init_from_raw_request(std::vector<uint8_t> raw_request);

private:

    Method m_method { Method::Invalid };
    std::vector<Header> m_headers;
    // std::vector<uint8_t> m_body;
    Url m_url;
    
};

} // namespace HTTP