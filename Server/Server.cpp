// Server.cpp: 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include <stdio.h>
#include <Winsock2.h>
#include <exception>
#include <iostream>
#include <cassert>
#include "requests.h"
#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 5208 //侦听端口

int bind_socket_listener(SOCKET& sListen)
{
  struct sockaddr_in saServer;
  saServer.sin_family = AF_INET; //地址家族
  saServer.sin_port = htons(SERVER_PORT); //注意转化为网络字节序
  saServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY); //使用INADDR_ANY 指示任意地址

  return bind(sListen, (struct sockaddr *)&saServer, sizeof(saServer));
}

SOCKET init_socket_listener()
{
  WSADATA wsaData;
  WORD wVersionRequested = MAKEWORD(2, 2); //希望使用的WinSock DLL 的版本
  if (WSAStartup(wVersionRequested, &wsaData) != 0)
  {
    printf("WSAStartup() failed!\n");
    return INVALID_SOCKET;
  }

  //创建Socket,使用TCP协议
  SOCKET sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sListen == INVALID_SOCKET)
  {
    WSACleanup();
    printf("socket() failed!\n");
    return INVALID_SOCKET;
  }

  if (bind_socket_listener(sListen) == SOCKET_ERROR)
  {
    printf("bind() faild! code:%d\n", WSAGetLastError());
    closesocket(sListen); //关闭套接字
    WSACleanup();
    return INVALID_SOCKET;
  }

  //侦听连接请求
  if (listen(sListen, 5) == SOCKET_ERROR)
  {
    printf("listen() faild! code:%d\n", WSAGetLastError());
    closesocket(sListen); //关闭套接字
    return INVALID_SOCKET;
  }

  return sListen;
}

void handle_client(SOCKET& sServer)
{
  while (true)
  {
    char receiveMessage[5000];
    //接收数据
    int ret = recv(sServer, receiveMessage, 5000, 0);
    if (ret == SOCKET_ERROR)
    {
      printf("recv() failed!\n");
      return;
    }
    if (ret == 0) //客户端已经关闭连接
    {
      printf("Client has closed the connection\n");
      break;
    }
    printf("receive message:%s\n", receiveMessage);//打印我们接收到的消息。
  }
}

void wait_for_new_clients(SOCKET sServer[2], SOCKET& sListen)
{
  struct sockaddr_in saClient;
  int length = sizeof(saClient);
  for (int i = 0; i < 2; ++i)
  {
    while (sServer[i] == INVALID_SOCKET)
    {
      sServer[i] = accept(sListen, (struct sockaddr *)&saClient, &length);
    }
  }
}

void wait_for_heartbeat(SOCKET& socket)
{
  char request_buffer[MAX_REQUEST_SIZE];
  if (!SOCKET_ACT_OK(recv(socket, request_buffer, MAX_REQUEST_SIZE, 0)))
  {
    throw std::exception("Receive request failed.");
  }
  Request *request = reinterpret_cast<Request*>(request_buffer);
  if (request->type_ != HEARTBEAT_REQUEST)
  {
    throw std::exception("Unexpected non-heartbeat request.");
  }
}

bool check_clients_ready(SOCKET sServer[2], SOCKET& sListen)
{
  for (int i = 0; i < 2; ++i)
  {
    assert(sServer[i] != INVALID_SOCKET);
    try
    {
      send_heartbeat(sServer[i]);
      wait_for_heartbeat(sServer[i]);
    }
    catch (const std::exception& e)
    {
      std::cerr << e.what() << std::endl;
      closesocket(sServer[i]);
      sServer[i] = INVALID_SOCKET;
      return false;
    }
  }
  return true;
}

void send_start_game_requests(SOCKET sServer[2])
{
  /* TODO: make is_offen random. */
  bool is_offen = true;
  for (int i = 0; i < 2; ++i)
  {
    /* TODO: check exception. */
    send_request(StartGameRequest(is_offen), sServer[i]);
    is_offen = !is_offen;
  }
}

void connect_player_pair(SOCKET sServer[2], SOCKET& sListen)
{
  do
  {
    wait_for_new_clients(sServer, sListen);
  } 
  while (!check_clients_ready(sServer, sListen));
  send_start_game_requests(sServer);
}

void main()
{
  SOCKET sListen = init_socket_listener();
  if (sListen == INVALID_SOCKET)
    return;

  std::cout << "Server is running!" << std::endl;

  SOCKET sServer[2] = { INVALID_SOCKET, INVALID_SOCKET };

  connect_player_pair(sServer, sListen);

  getchar();
  //  closesocket(sListen);
  //  closesocket(sServer);
  //  WSACleanup();
}