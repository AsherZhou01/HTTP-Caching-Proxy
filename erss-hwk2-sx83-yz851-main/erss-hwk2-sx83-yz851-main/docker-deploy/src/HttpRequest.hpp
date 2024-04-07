#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <iostream>
#include <string>
#include "Logger.hpp"

using std::iostream;
using std::string;

class Request
{

public:
    string requestMessage;
    string firstLine;
    string requestMethod;
    string hostName;
    string portNumber;
    string uriPath;

public:
    explicit Request(const string &request) : requestMessage(request)
    {
    }

    void parseRequest(int id);
};

#endif