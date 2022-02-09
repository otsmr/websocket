/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#include "HttpRequest.h"

namespace HTTP {

HttpRequest::Header HttpRequest::get_header (const std::string& name)
{
    for (const HTTP::HttpRequest::Header& h : m_headers)
        if (h.name == name)
            return h;

    Header res;
    return res;
}

HttpRequest HttpRequest::from_raw_request(std::vector<uint8_t> raw_request)
{

    enum class State {
        InMethod,
        InResource,
        InProtocol,
        InHeaderName,
        InHeaderValue,
    };

    State state = {
        State::InMethod
    };
    
    HttpRequest request;
    Header header;
    
    size_t index = 0;

    std::string method;
    std::string resource;
    std::string protocol;

    std::vector<uint8_t> buffer;

    auto commit = [&](auto& output, State new_state) {
        for (unsigned char & i : buffer)
            output.push_back(i);
        buffer.clear();
        state = new_state;
        index++;
    };

    auto at = [&](size_t offset = 0) -> uint8_t {
        if ((index + offset) >= raw_request.size())
            return 0;
        return raw_request.at(index + offset);
    };

    while (index < raw_request.size() ) {

        switch (state) {
            
            case State::InMethod:
                if (at() == ' ') 
                    commit(method, State::InResource);
                buffer.push_back(at());
                break;
            case State::InResource:
                if (at() == ' ')
                    commit(m_url.query, State::InProtocol);
                buffer.push_back(at());
                break;
            case State::InProtocol:
                if (at() == '\r')
                    index++;
                if (at() == '\n') {
                    commit(protocol, State::InHeaderName);
                } 
                buffer.push_back(at());
                break;
            case State::InHeaderName:
                if (at() == ':') {
                    index++;
                    header.name.clear();
                    header.value.clear();
                    commit(header.name, State::InHeaderValue);
                    transform(
                        header.name.begin(),
                        header.name.end(),
                        header.name.begin(),
                        ::tolower);
                }
                buffer.push_back(at());
                break;
            case State::InHeaderValue:
                if (at() == '\r')
                    index++;
                if (at() == '\n') {
                    commit(header.value, State::InHeaderName);
                    m_headers.push_back(header);
                    if (header.name == "host")
                        m_url.host = header.value;
                }
                buffer.push_back(at());
                break;
        }

        if (at() == '\r')
            index++;

        if (at() == '\n' && ((at(1) == '\n') || at(1) == '\r')) 
            break;

        index++;

    }

    std::string host;

    if (method == "GET")
        m_method = Method::GET;
    
    return request;

}

std::vector<std::string> HttpRequest::header_value_as_array(std::string name) {

    std::string value = get_header(name).value;
    std::string buffer;
    std::vector<std::string> array;

    for (size_t i = 0; i < value.size(); i++)
    {
        if (value.at(i) == ';') {
            array.push_back(buffer);
            buffer = "";
            continue;
        }
        buffer += value.at(i);
    }
    array.push_back(buffer);

    return array;

}


} // namespace HTTP