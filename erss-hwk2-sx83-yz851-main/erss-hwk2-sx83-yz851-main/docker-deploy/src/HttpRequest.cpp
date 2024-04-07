#include "HttpRequest.hpp"
#include "Logger.hpp"

string requestMessage;
string firstLine;
string requestMethod;
string hostName;
string portNumber;
string uriPath;

using namespace std;

void Request::parseRequest(int id)
{
    // 从requestMessage中提取第一行（直到\r\n），存储到firstLine
    firstLine = requestMessage.substr(0, requestMessage.find("\r\n"));

    size_t firstSpace = firstLine.find(' ');
    size_t secondSpace = firstLine.find(' ', firstSpace + 1);

    if (firstSpace == std::string::npos || secondSpace == std::string::npos)
    {
        cerr << "received incorrect request" << endl;
        writeLog(id, "ERROR Received incorrect request");
        return;
    }

    requestMethod = firstLine.substr(0, firstSpace);

    uriPath = firstLine.substr(firstSpace + 1, secondSpace - firstSpace - 1);

    try
    {
        size_t hostStart = requestMessage.find("Host:") + 6; // "Host: " has 6 characters
        if (hostStart == string::npos)
        {
            throw runtime_error("Host header not found");
        }

        string afterHost = requestMessage.substr(hostStart);
        size_t hostEnd = afterHost.find("\r\n");
        if (hostEnd == string::npos)
        {
            throw runtime_error("Invalid Host header format");
        }
        string hostLine = afterHost.substr(0, hostEnd);
        size_t portStart = hostLine.find(":");
        // 如果端口号存在，则提取端口号；如果不存在，默认端口号为80
        if (portStart != string::npos)
        {
            hostName = afterHost.substr(0, portStart);
            portNumber = hostLine.substr(portStart + 1);
        }
        else
        {
            hostName = hostLine;
            portNumber = "80";
        }
    }
    catch (const exception &e)
    {
        hostName = "";
        portNumber = "";
    }
}
