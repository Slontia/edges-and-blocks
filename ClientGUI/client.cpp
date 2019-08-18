#pragma once

// Client.cpp: 定义控制台应用程序的入口点。
//

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <iostream>
#include <exception>
#include "client.h"
#pragma comment(lib, "ws2_32.lib")

Client::Client() : sClient_(init_socket())
{
  if (sClient_ == INVALID_SOCKET)
  {
    throw std::exception("Client boot failed.");
  }
}

Client::~Client()
{
  if (sClient_ != INVALID_SOCKET)
  {
    closesocket(sClient_);
  }
  WSACleanup();
}

SOCKET Client::init_socket()
{
  WORD wVersionRequested;
  WSADATA wsaData;
  int ret;
  SOCKET sClient; //连接套接字
  struct sockaddr_in saServer; //地址信息
  BOOL fSuccess = TRUE;

  //WinSock初始化
  wVersionRequested = MAKEWORD(2, 2); //希望使用的WinSock DLL的版本
  if (WSAStartup(wVersionRequested, &wsaData) != 0)
  {
    printf("WSAStartup() failed!\n");
    return INVALID_SOCKET;
  }
  //确认WinSock DLL支持版本2.2
  if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
  {
    WSACleanup();
    printf("Invalid WinSock version!\n");
    return INVALID_SOCKET;
  }

  //创建Socket,使用TCP协议
  sClient = socket(AF_INET, SOCK_STREAM, 0);
  if (sClient == INVALID_SOCKET)
  {
    WSACleanup();
    printf("socket() failed!\n");
    return INVALID_SOCKET;
  }

  //构建服务器地址信息
  saServer.sin_family = AF_INET; //地址家族
  saServer.sin_port = htons(SERVER_PORT); //注意转化为网络节序
  saServer.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

  //连接服务器
  ret = connect(sClient, (struct sockaddr *)&saServer, sizeof(saServer));
  if (ret == SOCKET_ERROR)
  {
    printf("connect() failed!\n");
    closesocket(sClient); //关闭套接字
    WSACleanup();
    return INVALID_SOCKET;
  }

  return sClient;
}


bool Client::wait_for_game_start()
{
  char request_buffer[MAX_REQUEST_SIZE];
  Request *request = nullptr;
  while (true)
  {
    if (!SOCKET_ACT_OK(recv(sClient_, request_buffer, MAX_REQUEST_SIZE, 0)))
    {
      throw std::exception("Server closed.");
    }
    request = reinterpret_cast<Request*>(request_buffer);
    if (request->type_ == HEARTBEAT_REQUEST)
    {
      send_heartbeat(sClient_);
    }
    else
    {
      break;
    }
  }
  if (request->type_ != START_GAME_REQUEST)
  {
    throw std::exception("Unexpected request from server before game start.");
  }
  return reinterpret_cast<StartGameRequest*>(request)->is_offen_;
}

Request& Client::receive_request()
{
  memset(request_buffer_, 0, MAX_REQUEST_SIZE);
  recv(sClient_, request_buffer_, MAX_REQUEST_SIZE, 0);
  return *reinterpret_cast<Request*>(request_buffer_);
}

ClientWorker::ClientWorker() : client_(std::make_unique<Client>()) {}

void ClientWorker::wait_for_game_start()
{
  const bool& is_offen = client_->wait_for_game_start();
  emit game_started(is_offen);
}

void ClientWorker::receive_request()
{
  const Request& request = client_->receive_request();
  emit request_received(request);
}
  
ClientAsyncWrapper::ClientAsyncWrapper() : worker_(new ClientWorker())
{
  connect(this, SIGNAL(wait_for_game_start()), worker_, SLOT(ClientWorker::wait_for_game_start()));
  connect(this, SIGNAL(receive_request()), worker_, SLOT(ClientWorker::receive_request()));
  connect(worker_, SIGNAL(ClientWorker::game_started(const bool&)), this, SLOT(exec_game_started_callback()));
  connect(worker_, SIGNAL(ClientWorker::request_received(const Request&)), this, SLOT(exec_request_received_callback(const Request&)));

  worker_->moveToThread(&thread_);
  connect(&thread_, SIGNAL(QThread::finished()), worker_, SIGNAL(deleteLater()));
  thread_.start();
}

ClientAsyncWrapper::~ClientAsyncWrapper()
{
  thread_.quit();
  thread_.wait();
}

/* Does NOT thread safe. */
void ClientAsyncWrapper::wait_for_game_start_async(GameStartedCallback f)
{
  game_started_f = f;
  emit wait_for_game_start();
}

/* Does NOT thread safe. */
void ClientAsyncWrapper::receive_request_async(RequestReceivedCallback f)
{
  req_recv_f = f;
  emit receive_request();
}
