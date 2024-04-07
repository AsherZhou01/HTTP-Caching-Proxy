#ifndef PROXYSERVER_HPP
#define PROXYSERVER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpCache.hpp"
#include "Logger.hpp"

using std::string;
using std::vector;

void processConnectRequest(int id, int server_fd, int client_fd);
extern void *manageClientRequest(void *connectionInfo);
////extern void processConnection(int connID, int serverSocket, int clientSocket, Request httpRequest, DataCache& dataCache);
extern void handleDataRetrieval(int fileDescriptor, vector<char> &buffer);
extern void processGETRequest(int connID, int serverSock, int clientSock, Request httpRequest, DataCache &dataCache);
string fetchAndCacheResponse(int id, int server_fd, Request &requestToServer, DataCache &cache, Response cachedResponse);
void receiveCompleteResponse(int fd, vector<char> &msg_vec, string requestMethod);
void processPOSTRequest(int id, int server_fd, int client_fd, Request clientRequest);
string getNewResponse(int id, int server_fd, DataCache &cache, Response cachedResponse, Request originRequest);
#endif