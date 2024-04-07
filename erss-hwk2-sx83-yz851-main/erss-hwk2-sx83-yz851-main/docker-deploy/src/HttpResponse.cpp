#include "HttpResponse.hpp"
using namespace std;
#include "Logger.hpp"
#include <string>

bool Response::needRevalidation(int id)
{
    string maxAge_value = "";
    // Check for no-cache directive first
    // bool hasNoCache = false;
    string CacheControlValue = find_str("Cache-Control");

    if (!CacheControlValue.empty())
    {
        size_t noCachePos = CacheControlValue.find("no-cache");
        size_t maxAgePos = CacheControlValue.find("max-age");
        if (noCachePos != string::npos)
        {
            // hasNoCache = true;
            writeLog(id, "in cache, requires validation");
            return true;
        }
        if (maxAgePos != string::npos)
        {
            size_t maxAgeValuePos = maxAgePos + 8;
            size_t maxAgeValueEnd = CacheControlValue.find(",", maxAgeValuePos + 1);
            if (maxAgeValueEnd == string::npos)
            {
                maxAgeValueEnd = CacheControlValue.find("\r\n", maxAgeValuePos + 1);
            }
            maxAge_value = CacheControlValue.substr(maxAgeValuePos, maxAgeValueEnd - maxAgeValuePos);
        }
    }

    // Check if the response is new based on Expires, Last-Modified, and max-age
    string expiresValue = find_str("Expires");
    time_t expiresSecond = fetch_utc_timestamp(expiresValue);
    time_t currentSecond = time(NULL) - 4 * 3600;
    // writeLog(id, "expiresValue");
    // writeLog(id, expiresValue);
    // writeLog(id, "expiresValue");
    if (!expiresValue.empty())
    {
        if (currentSecond < expiresSecond)
        {
            return false;
        }
        else
        {
            writeLog(id, "in cache, but expired at " + expiresValue);
            return true;
        }
    }
    // string lastModify_value = find_str("Last-Modified");
    // check max age
    currentSecond += 4 * 3600;
    if (!maxAge_value.empty())
    {

        int maxAgeNum = std::atoi(maxAge_value.c_str());
        // writeLog(id, std::to_string(maxAgeNum));
        // writeLog(id, std::to_string(difftime(currentSecond, fetch_utc_timestamp(find_str("Date")))));
        if (maxAgeNum > difftime(currentSecond, fetch_utc_timestamp(find_str("Date"))))
        {
            return false;
        }
        else
        {
            writeLog(id, "in cache, but past max-age of " + maxAge_value + " seconds");
            return true;
        }
    }
    else
    {
        // writeLog(id, "in cache, but past max-age of " + maxAge_value + " seconds");
        return true;
    }
    return false;
}
