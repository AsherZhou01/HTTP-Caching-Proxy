#include "ProxyServer.hpp"

#include <thread>
#include <iostream>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <unistd.h>
#include "main.hpp"
// Assuming this includes necessary headers and declarations.
#define BUFFER_SIZE 4096

void *manageClientRequest(void *connectionInfo)
{
  ConnectionDetails *details = static_cast<ConnectionDetails *>(connectionInfo);

  char requestBuffer[BUFFER_SIZE] = {0};
  ssize_t bytesRead = recv(details->connectionFD, requestBuffer, sizeof(requestBuffer), 0);
  if (bytesRead < 0)
  {
    std::cerr << "[error] Invalid or empty request received." << std::endl;
    return nullptr;
  }
  // 封装成Request对象
  Request httpRequest(std::string(requestBuffer, bytesRead));
  httpRequest.parseRequest(details->connectionID);

  // writeLog(details->connectionID, httpRequest.firstLine + " from " + details->clientIP + "@" + formatCurrentTime());
  // writeLog(details->connectionID, httpRequest.requestMethod);
  // writeLog(details->connectionID, httpRequest.hostName);
  // writeLog(details->connectionID, httpRequest.portNumber);
  // writeLog(details->connectionID, httpRequest.uriPath);

  //  string requestMessage;
  //   string firstLine;
  //   string requestMethod;
  //   string hostName;
  //   string portNumber;
  //   string uriPath;
  struct addrinfo hints, *serverInfo;
  int webServerSocket;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if (getaddrinfo(httpRequest.hostName.c_str(), httpRequest.portNumber.c_str(), &hints, &serverInfo) != 0)
  {
    std::cerr << "[error] Unable to resolve web server address for " << httpRequest.hostName << "." << std::endl;
    return nullptr;
  }

  webServerSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
  if (webServerSocket < 0)
  {
    std::cerr << "[error] Failed to create socket to web server." << std::endl;
    freeaddrinfo(serverInfo);
    return nullptr;
  }

  if (connect(webServerSocket, serverInfo->ai_addr, serverInfo->ai_addrlen) < 0)
  {
    std::cerr << "[error] Failed to connect to " << httpRequest.hostName << " on port " << httpRequest.portNumber << "." << std::endl;
    close(webServerSocket);
    freeaddrinfo(serverInfo);
    return nullptr;
  }
  writeLog(details->connectionID, "Connection established to " + httpRequest.hostName + " on port " + httpRequest.portNumber);
  std::cout << "Connection established to " << httpRequest.hostName << " on port " << httpRequest.portNumber << "." << std::endl;
  freeaddrinfo(serverInfo); // Free the server info after successful connection

  // Proceed with handling the request based on its type (CONNECT, GET, POST)
  if (httpRequest.requestMethod == "CONNECT")
  {
    processConnectRequest(details->connectionID, webServerSocket, details->connectionFD);
    writeLog(details->connectionID, "Tunnel closed.");
  }
  else if (httpRequest.requestMethod == "GET")
  {
    // writeLog(details->connectionID, "before handle get.");
    processGETRequest(details->connectionID, webServerSocket, details->connectionFD, httpRequest, *details->dataCache);
  }
  else if (httpRequest.requestMethod == "POST")
  {
    processPOSTRequest(details->connectionID, webServerSocket, details->connectionFD, httpRequest);
  }

  close(webServerSocket); // Ensure the socket to the web server is closed after handling the request
  return nullptr;
}

string fetchAndCacheResponse(int id, int server_fd, Request &requestToServer, DataCache &cache, Response cachedResponse)
{

  send(server_fd, requestToServer.requestMessage.data(), requestToServer.requestMessage.size() + 1, 0);
  writeLog(id, "Requesting " + requestToServer.firstLine + " from " + requestToServer.hostName);
  vector<char> receivedResponse;

  receiveCompleteResponse(server_fd, receivedResponse, requestToServer.requestMethod);

  string response_str(receivedResponse.begin(), receivedResponse.end());
  Response newResponse(response_str);
  writeLog(id, "Received " + newResponse.statusLine + " from " + requestToServer.hostName);

  return cache.updateCache(id, requestToServer, newResponse, cachedResponse);

  // 返回从服务器获取的响应字符串
}

string getNewResponse(int id, int server_fd, DataCache &cache, Response cachedResponse, Request originRequest)
{
  string etag = cachedResponse.find_str("ETag");
  string last_mod = cachedResponse.find_str("Last-Modified");
  string newRequestMessage;
  if (last_mod != "")
  {
    // send new req to web server
    newRequestMessage = originRequest.firstLine + "\r\n" + "If-Modified-Since: " + last_mod + "\r\n\r\n";
    // send(server_fd,req_new.data(),req_new.size()+1,0);
    // return try_validate(id, server_fd, req,res);
  }
  else if (etag != "")
  {
    newRequestMessage = originRequest.firstLine + "\r\n" + "If-None-Match: " + etag + "\r\n\r\n";
    // send(server_fd,req_new.data(),req_new.size()+1,0);
    // return try_validate(id, server_fd, req,res);
  }

  else
  {
    // resend req to the server
    cout << "[info] resend request to " << server_fd << endl;
    newRequestMessage = originRequest.requestMessage;

    // send(server_fd,origin.data(),origin.size()+1,0);
    // vector<char> recv_vec;
    // recv_complete(server_fd,recv_vec);

    // string response_str;
    // response_str.insert(response_str.begin(), recv_vec.begin(), recv_vec.end());
    // Response newResponse(response_str);
    // try_save_cache(id, req.uri,newResponse);
    // return newResponse.complete_message;
  }
  Request newRequest(newRequestMessage);
  newRequest.parseRequest(id);

  return fetchAndCacheResponse(id, server_fd, newRequest, cache, cachedResponse);
}

void processGETRequest(int id, int server_fd, int client_fd, Request clientRequest, DataCache &cache)
{
  // writeLog(id, "in GET handle111");
  string cachedResponseMessage = cache.retrieveCachedResponse(clientRequest.uriPath);
  Response cachedResponse(cachedResponseMessage);
  // writeLog(id, "in GET handle");
  if (cachedResponseMessage != "")
  {
    cout << "[info] get response in cache" << endl;
    // writeLog(id, "[info] get response in cache");
    // Response response(cachedResponse);
    if (cachedResponse.needRevalidation(id))
    {
      cachedResponseMessage = getNewResponse(id, server_fd, cache, cachedResponse, clientRequest);
    }
    else
    {
      writeLog(id, "in cache, valid");
    }
  }
  else
  {
    // 直接使用 fetchAndCacheResponse 函数的返回值更新响应字符串
    cout << "[info] get response not in cache" << endl;
    writeLog(id, "not in cache");
    cachedResponseMessage = fetchAndCacheResponse(id, server_fd, clientRequest, cache, cachedResponse);
  }
  // writeLog(-1,cachedResponseMessage );
  Response newResponse(cachedResponseMessage);
  send(client_fd, cachedResponseMessage.data(), cachedResponseMessage.size() + 1, 0);
  // writeLog(-1,cachedResponse.responseMessage  );

  writeLog(id, "Responding " + newResponse.statusLine + "\r\n");
}

void processPOSTRequest(int id, int server_fd, int client_fd, Request clientRequest)
{

  string requestMessage = clientRequest.requestMessage;
  size_t contentLenthPos = requestMessage.find("Content-Length: ");
  if (contentLenthPos != std::string::npos)
  {
    // size_t contentLenthValuePos=contentLenthPos+16;
    // size_t contentLenthValueEnd= requestMessage.find("\r\n",contentLenthValuePos);
    // string contentLenthValue = requestMessage.substr(contentLenthValuePos,contentLenthValueEnd-contentLenthValuePos);
    // int contentLenthNum = stoi(contentLenthValue);

    send(server_fd, clientRequest.requestMessage.data(), clientRequest.requestMessage.size() + 1, 0);

    writeLog(id, "Requesting " + clientRequest.firstLine + " from " + clientRequest.hostName);
    vector<char> receivedResponse;

    receiveCompleteResponse(server_fd, receivedResponse, clientRequest.requestMethod);
    // int rsp_len = recv(server_fd, response_char, sizeof(response_char), MSG_WAITALL);
    string response_str(receivedResponse.begin(), receivedResponse.end());
    Response newResponse(response_str);
    writeLog(id, "Received " + newResponse.statusLine + " from " + clientRequest.hostName);

    send(client_fd, response_str.data(), response_str.size(), 0);
    writeLog(id, "Received " + newResponse.statusLine + " from " + clientRequest.hostName);
    cout << "[info] proxy handle post success" << endl;
  }
  else
  {
    cerr << "[error] cannot post with socket " << client_fd << endl;
    return;
  }
}

// string fetchAndCacheResponse(int id, int server_fd, Request &requestToServer, DataCache &cache) {

//     send(server_fd, requestToServer.requestMessage.data(), requestToServer.requestMessage.size() + 1, 0);
//     writeLog(id, "Requesting " + req.line + " from " + req.hostname);
//     vector<char> recv_vec;

//     recv_complete(server_fd, recv_vec,requestToServer.requestMethod);

//     string response_str(recv_vec.begin(), recv_vec.end());
//     Response rsp(response_str);
//     writeLog(id, "Received " + rsp.statusLine + " from " + req.hostname);

//     cache.try_save_cache(id, req.uri, rsp);

//     return response_str; // 返回从服务器获取的响应字符串
// }

void forward_data(int source_sock, int destination_sock)
{
  char buffer[BUFFER_SIZE];
  ssize_t bytes_read = recv(source_sock, buffer, BUFFER_SIZE, 0);
  if (bytes_read > 0)
  {
    send(destination_sock, buffer, bytes_read, 0);
  }
}
void processConnectRequest(int id, int server_fd, int client_fd)
{
  const char *message = "HTTP/1.1 200 Connection Established\r\n\r\n";
  send(client_fd, message, strlen(message), 0);
  writeLog(id, "Responding HTTP/1.1 200 OK\r\n\r\n");

  fd_set readfds;
  int max_sd;
  while (true)
  {
    FD_ZERO(&readfds);
    FD_SET(client_fd, &readfds);
    FD_SET(server_fd, &readfds);
    max_sd = (client_fd > server_fd ? client_fd : server_fd);

    // 等待活动的socket
    int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

    if ((activity < 0) && (errno != EINTR))
    {
      printf("select error");
    }

    // 如果客户端socket是活动的，转发到服务器
    if (FD_ISSET(client_fd, &readfds))
    {
      forward_data(client_fd, server_fd);
    }

    // 如果服务器socket是活动的，转发到客户端
    if (FD_ISSET(server_fd, &readfds))
    {
      forward_data(server_fd, client_fd);
    }
  }
}

void receiveCompleteResponse(int fd, vector<char> &msg_vec, string requestMethod)
{

  msg_vec.resize(BUFFER_SIZE);

  size_t len = recv(fd, &msg_vec.data()[0], msg_vec.size(), 0);
  if (len <= 0)
  {
    cerr << "[error] cannot recv in handle get" << endl;
    return;
  }

  string res_str;
  res_str.insert(res_str.begin(), msg_vec.begin(), msg_vec.end());
  Response res_obj = Response(res_str);

  size_t i = len;

  string ChunkedHead = res_obj.find_str("Transfer-Encoding");

  //  string requestMessage= clientRequest.requestMessage;
  //   size_t contentLenthPos= requestMessage.find("Content-Length: ");
  //   if(contentLenthPos!= std::string::npos){
  // size_t contentLenthValuePos=contentLenthPos+16;
  // size_t contentLenthValueEnd= requestMessage.find("\r\n",contentLenthValuePos);
  // string contentLenthValue = requestMessage.substr(contentLenthValuePos,contentLenthValueEnd-contentLenthValuePos);
  // int contentLenthNum = stoi(contentLenthValue);v

  if (ChunkedHead.find("chunked") == string::npos)
  {
    string contentLenthValue = res_obj.find_str("Content-Length");
    int contentLenthNum = stoi(contentLenthValue);
    cout << "[info] received response which is not chunked, with con_len: " << len << endl;
    while (i < contentLenthNum)
    {
      msg_vec.resize(i + BUFFER_SIZE);
      len = recv(fd, &msg_vec.data()[i], msg_vec.size(), 0);

      if (len <= 0)
      {
        cerr << "[info] proxy failed to handle the request method " << requestMethod << endl;
        return;
      }
      i = i + len;
    }
  }

  else
  {
    cout << "[info] received response which is chunked, no con_len" << endl;
    string recv_str;
    recv_str.insert(recv_str.begin(), msg_vec.begin(), msg_vec.end());
    while (recv_str.find("0\r\n\r\n") == string::npos)
    {
      msg_vec.resize(i + BUFFER_SIZE);
      len = recv(fd, &msg_vec.data()[i], BUFFER_SIZE, 0);

      if (len <= 0)
      {
        cerr << "[info] proxy failed to handle the request method " << requestMethod << endl;
        break;
      }
      recv_str = "";
      recv_str.insert(recv_str.begin(), msg_vec.begin() + i, msg_vec.begin() + len + i);
      i += len;
    }
    i = i + len;
  }
  cout << "[info] proxy handle the request method " << requestMethod
       << " successfully" << endl;
}
