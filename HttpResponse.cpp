/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include "HttpResponse.h"

#include <utility>

namespace HTTP {

HttpResponse::HttpResponse()
= default;
HttpResponse::~HttpResponse()
= default;

HttpResponse::Header HttpResponse::set_header (std::string name, std::string value) {
    Header h = {
        std::move(name),
        std::move(value)
    };
    m_headers.push_back(h);
    return h;
}

std::vector<uint8_t> HttpResponse::get_raw_response() {

    std::string response;

    response += m_protocol + " " + std::to_string(m_statuscode) + " ";

    switch (m_statuscode)
    {
    case Statuscode::SwitchingProtocols:
        response += "Switching Protocols" ;
        break;
    case Statuscode::BadRequest:
        response += "Bad Request";
        break;
    default:
        break;
    }
    response += linebreak;
    
    for (const HTTP::HttpResponse::Header& header : m_headers) 
        response += header.name + ": " + header.value + linebreak;

    response += linebreak;

    std::vector<uint8_t> raw_response(response.begin(), response.end());
    raw_response.assign(response.begin(), response.end());

    return raw_response;

}

} // namespace HTTP