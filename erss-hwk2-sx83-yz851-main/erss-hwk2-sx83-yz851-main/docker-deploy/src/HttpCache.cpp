#include <string>
#include "HttpCache.hpp"
#include <pthread.h>
#include <sys/socket.h>
#include <mutex>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>
using namespace std;
mutex mutex1;
pthread_mutex_t cacheMutex;

std::string DataCache::retrieveCachedResponse(const std::string &key)
{
    // writeLog(id, "in find cache0");
    lock_guard<mutex> lck(mutex1);
    //    pthread_mutex_lock(&cacheMutex);
    // std::unordered_map<std::string, std::string>::iterator it = cacheStorage.find(key); // Explicit iterator type
    //    pthread_mutex_unlock(&cacheMutex);
    //    writeLog(-1, "in find cache 1");
    // if (it != cacheStorage.end())
    // {
    //     std::string result = it->second; // If found, store the associated value

    //     return result;
    // }
    if (cacheStorage.count(key) != 0)
    {
        // writeLog(-1, "in find cache 1");
        return cacheStorage[key];
    }

    return ""; // Return empty string if not found
}

// std::string DataCache::revalidateCachedResponse(int requestId, int serverSocket, Response existingResponse, Request request) {
//     std::string etag = existingResponse.find_str("ETag");
//     std::string lastModified = existingResponse.find_str("Last-Modified");
//     size_t headerEndPos = request.requestMessage.find("\r\n\r\n");
//     std::string requestHeader = request.requestMessage.substr(0, headerEndPos);
//     if (lastModified != "") {
//         // Send new request with If-Modified-Since header to the web server
//         std::string newRequest = requestHeader + "\r\nIf-Modified-Since: " + lastModified + "\r\n\r\n";
//         send(serverSocket, newRequest.data(), newRequest.size() + 1, 0);
//     } else if (etag != "") {
//         // Send new request with If-None-Match header to the web server
//         std::string newRequest = requestHeader + "\r\nIf-None-Match: " + etag + "\r\n\r\n";
//         send(serverSocket, newRequest.data(), newRequest.size() + 1, 0);
//     } else {
//         // Resend the original request to the server
//         std::cout << "[info] Resend request to " << serverSocket << std::endl;
//         std::string originalRequest = request.requestMessage;
//         send(serverSocket, originalRequest.data(), originalRequest.size() + 1, 0);
//     }

//     // Regardless of which path was taken, we now need to validate and potentially update the cache.
//     // We use try_validate for this, which will handle the response from the server.
//     return updateCache(requestId, serverSocket, request, existingResponse);
// }

std::string DataCache::updateCache(int requestId, Request request, Response newResponse, Response cachedResponse)
{
    // std::vector<char> recv_vec;
    // recv_complete(serverSocket, recv_vec, request.requestMethod); // Assuming this function is correctly defined elsewhere

    // std::string response_str;
    // response_str.insert(response_str.begin(), recv_vec.begin(), recv_vec.end());
    // // Response newResponse(response_str); // This might need to be adjusted if Response cannot be directly constructed from a string
    ///////////////////////------------------》
    int statusCodePos = newResponse.statusLine.find(' ') + 1;
    int statusCodeEnd = newResponse.statusLine.find(' ', statusCodePos);
    std::string statusCode = newResponse.statusLine.substr(statusCodePos, statusCodeEnd - statusCodePos);
    // 假设expiresStr是从HTTP响应头中获取的Expires值，例如："Fri, 29 Oct 2021 19:43:31 GMT"
    std::string expiresStr = newResponse.find_str("Expires");

    // 将Expires字符串转换为time_t
    std::tm tm = {};
    std::stringstream ss(expiresStr);
    ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
    auto expiresTime = std::mktime(&tm);
    // 初始化noStorePos和privatePos为string::npos
    size_t noStorePos = string::npos;
    size_t privatePos = string::npos;

    if (statusCode != "304")
    {   if(statusCode == "502"){
   
    std::string modifiedResponse = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    return modifiedResponse;
}else{ string CacheControl = newResponse.find_str("Cache-Control");
        if (!CacheControl.empty())
        {
            noStorePos = CacheControl.find("no-store");
            privatePos = CacheControl.find("private");
        }

        // 只有当状态码为200且没有no-store或private时，才认为是可缓存的
        bool isAbleToCache = (noStorePos == string::npos) && (privatePos == string::npos) && (statusCode == "200");

        if (!isAbleToCache)
        {
            if (noStorePos != string::npos)
            {
                writeLog(requestId, "not cacheable because: Cache-control is no-store");
            }
            else if (privatePos != string::npos)
            {
                writeLog(requestId, "not cacheable because: Cache-control is private");
            }
        }
        else
        {
            pthread_mutex_lock(&cacheMutex);
            cacheStorage[request.uriPath] = newResponse.responseMessage; // 保存完整的响应信息而不只是状态行
            pthread_mutex_unlock(&cacheMutex);

            std::cout << "[cache] after insert new value to map" << std::endl;
            for (auto it = cacheStorage.begin(); it != cacheStorage.end(); ++it)
            {
                std::cout << "-----" << it->first << "---" << it->second << std::endl;
            }
            // 获取当前时间
            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

            if (newResponse.find_str("Expires") != "")
            {
                if (now > expiresTime)
                {
                    // 缓存已过期
                    time_t epochTime = 0;
                    if (expiresTime == epochTime)
                    {
                        writeLog(requestId, "not in cache!");
                    }
                    else
                    {
                        writeLog(requestId, "in cache, requires validation");
                    }
                }
                else
                {
                    // 缓存仍有效
                    writeLog(requestId, "cached, expires at " + newResponse.find_str("Expires"));
                }
            }
            else
            {
                writeLog(requestId, "cached, but requires re-validation");
            }
        }

        return newResponse.responseMessage;
    }
}

       
    return cachedResponse.responseMessage;

    // int statusCodePos = newResponse.statusLine.find(' ') + 1;
    // int statusCodeEnd = newResponse.statusLine.find(' ', statusCodePos);
    // std::string statusCode = newResponse.statusLine.substr(statusCodePos, statusCodeEnd - statusCodePos);
    // size_t noStorePos;
    // size_t privatePos;
    // if (statusCode != "304")
    // {
    //     string CacheControlPos = newResponse.find_str("Cache-Control");
    //     if (CacheControlPos != "")
    //     {
    //         noStorePos = CacheControlPos.find("no-store");
    //         privatePos = CacheControlPos.find("private");
    //     }
    //     bool isAbleToCache = (noStorePos == string::npos) && (privatePos == string::npos) && statusCode == "200";
    //     if (!isAbleToCache)
    //     {
    //         if (noStorePos != string::npos)
    //         {
    //             writeLog(requestId, "not cacheable because: Cache-control is no-store");
    //         }
    //         else if (privatePos != string::npos)
    //         {
    //             writeLog(requestId, "not cacheable because: Cache-control is private");
    //         }
    //     }
    //     else
    //     {
    //         pthread_mutex_lock(&cacheMutex);
    //         cacheStorage[request.uriPath] = newResponse.statusLine;
    //         pthread_mutex_unlock(&cacheMutex);

    //         std::cout << "[cache] after insert new value to map" << std::endl;
    //         for (std::map<std::string, std::string>::const_iterator it = cacheStorage.begin(); it != cacheStorage.end(); ++it)
    //         {
    //             std::cout << "-----" << it->first << "---" << it->second << std::endl;
    //         }

    //         if (newResponse.find_str("Expires") != "")
    //         {
    //             writeLog(requestId, "cached, expires at " + newResponse.find_str("Expires"));
    //         }
    //         else
    //         {
    //             writeLog(requestId, "cached, but requires re-validation");
    //         }
    //     }

    //     return newResponse.responseMessage;
    // }

    // return cachedResponse.responseMessage;
}
