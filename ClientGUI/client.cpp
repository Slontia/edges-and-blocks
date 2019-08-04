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

#define SERVER_PORT 5209 //侦听端口


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
  ret = WSAStartup(wVersionRequested, &wsaData);
  if (ret != 0)
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
  sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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


void Client::wait_for_game_start()
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
  is_offen_ = reinterpret_cast<StartGameRequest*>(request)->is_offen_;
}

Request& Client::receive_request()
{
  recv(sClient_, request_buffer_, MAX_REQUEST_SIZE, 0);
  return *reinterpret_cast<Request*>(request_buffer_);
}

bool Client::is_offen() { return is_offen_; }


/*
void main()
{
  std::unique_ptr<Client> client;
  try
  {
    client = std::make_unique<Client>();
    client->wait_for_game_start();
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return;
  }
  std::cout << "Game Start!" << std::endl;
}*/

