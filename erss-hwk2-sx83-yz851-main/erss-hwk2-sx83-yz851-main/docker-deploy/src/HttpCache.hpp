#ifndef HTTPCACHE_HPP
#define HTTPCACHE_HPP

#include <map>

#include "Logger.hpp"
// #include "ProxyServer.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

// Forward declarations to resolve circular dependencies, if necessary.

class DataCache
{
public:
  map<string, string> cacheStorage;

public:
  // Tries to validate a cached response with the origin server.// Tries to validate a cached response with the origin server.
  std::string updateCache(int requestId, Request request, Response newResponse, Response cachedResponse);
  std::string retrieveCachedResponse(const std::string &key);
  // Retrieves a cached response based on the request URI.

  // Revalidates a cached response, potentially fetching a new one.
  // std::string revalidateCachedResponse(int requestId, int serverSocket, Response existingResponse, Request request);
};

#endif // DATA_CACHE_H
