#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP
#include "Logger.hpp"
#include <string>
#include <ctime>

class Response
{
public:
    std::string responseMessage;
    // HTTP响应的状态行，包括HTTP版本、状态码和状态消息
    std::string statusLine; // Renamed from first_line to status_line
public:
    bool needRevalidation(int id);

    // Constructor adjusted for C++98
    Response(const std::string &response)
    {
        responseMessage = response;
        // Calculate position in a C++98 compliant manner
        std::string::size_type newlinePos = responseMessage.find("\n");
        if (newlinePos != std::string::npos)
        {
            statusLine = responseMessage.substr(0, newlinePos - 1);
        }
        else
        {
            statusLine = "";
        }
    }

    time_t fetch_utc_timestamp(const std::string &input_time)
    {
        std::tm time_structure = {};
        std::size_t position_of_gmt = input_time.find("GMT");
        if (position_of_gmt != std::string::npos)
        {

            time_structure.tm_isdst = -1;
        }
        std::string adjusted_time = input_time;
        if (position_of_gmt != std::string::npos)
        {
            adjusted_time.erase(position_of_gmt - 1, 4);
        }

        strptime(adjusted_time.c_str(), "%a, %d %b %Y %H:%M:%S", &time_structure);

        return timegm(&time_structure);
    }

    // bool check_str(const std::string &str);
    string find_str(string str)
    {
        size_t begin = responseMessage.find(str);
        if (begin != string::npos)
        {
            size_t end = responseMessage.find("\r\n", begin + str.size());
            size_t result_begin = begin + str.size() + 2;
            return responseMessage.substr(result_begin, end - result_begin);
        }
        return "";
    }

    // std::string get_http_code();
};

#endif