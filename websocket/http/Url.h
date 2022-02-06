/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#pragma once

namespace HTTP {

class Url {
public:
    std::string protocol = "http://";
    std::string host;
    std::string path;
    std::string query;
};

} // namespace HTTP