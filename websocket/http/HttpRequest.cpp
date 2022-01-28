/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */


#include "HttpRequest.h"

namespace HTTP {
    
HttpRequest::HttpRequest()
{
}
HttpRequest::~HttpRequest()
{
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

    State state = { State::InMethod };
    HttpRequest request;
    size_t index = 0;

    std::string method = "";
    std::vector<uint8_t> buffer;


    auto commit = [&](auto& output, State new_state) {

        for (int i=0;  i < buffer.size(); i++)
            output.push_back(buffer.at(i));

        buffer.clear();
        state = new_state;

    };

    while (index < raw_request.size() ) {

        switch (state) {
            case State::InMethod:
                if (raw_request.at(index) == ' ') {
                    commit(method, State::InResource);
                }
                buffer.push_back(raw_request.at(index));
                break;
        }

        index++;
    }

    if (method == "GET")
        m_method = Method::GET;

    std::cout << "method=" << method << "\n";
    printf("\nMethod: %d",  Method::GET);

    return request;
}

} // namespace HTTP