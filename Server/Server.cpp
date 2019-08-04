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

#define SERVER_PORT 5209 //侦听端口

class Server
{

public:

  Server() : sListen_(init_socket_listener())
  {
    if (sListen_ == INVALID_SOCKET)
    {
      throw std::exception("Server boot failed.");
    }
  }

  void connect_player_pair(SOCKET sServer[2])
  {
    do
    {
      wait_for_new_clients(sServer, sListen_);
    } while (!check_clients_ready(sServer, sListen_));
    send_start_game_requests(sServer);
  }

  void keep_deliver_request(SOCKET sServer[2])
  {
    char request_buffer[MAX_REQUEST_SIZE];
    while (true)
    {
      int size = recv(sServer[0], request_buffer, MAX_REQUEST_SIZE, 0);
      if (!SOCKET_ACT_OK(size) || !SOCKET_ACT_OK(send(sServer[1], request_buffer, size, 0)))
      {
        /* TODO: handle one player disconnect */
        break;
      }
    }
  }

private:
  SOCKET sListen_;

  int bind_socket_listener()
  {
    struct sockaddr_in saServer;
    saServer.sin_family = AF_INET; //地址家族
    saServer.sin_port = htons(SERVER_PORT); //注意转化为网络字节序
    saServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY); //使用INADDR_ANY 指示任意地址

    return bind(sListen_, (struct sockaddr *)&saServer, sizeof(saServer));
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
    sListen_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sListen_ == INVALID_SOCKET)
    {
      WSACleanup();
      printf("socket() failed!\n");
      return INVALID_SOCKET;
    }

    if (bind_socket_listener() == SOCKET_ERROR)
    {
      printf("bind() faild! code:%d\n", WSAGetLastError());
      closesocket(sListen_); //关闭套接字
      WSACleanup();
      return INVALID_SOCKET;
    }

    //侦听连接请求
    if (listen(sListen_, 5) == SOCKET_ERROR)
    {
      printf("listen() faild! code:%d\n", WSAGetLastError());
      closesocket(sListen_); //关闭套接字
      return INVALID_SOCKET;
    }

    return sListen_;
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

  static void wait_for_new_clients(SOCKET sServer[2], SOCKET& sListen)
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

  static void wait_for_heartbeat(SOCKET& socket)
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

  static bool check_clients_ready(SOCKET sServer[2], SOCKET& sListen)
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

  static void send_start_game_requests(SOCKET sServer[2])
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
};


void main()
{
  Server server;

  std::cout << "Server is running!" << std::endl;

  SOCKET sServer[2] = { INVALID_SOCKET, INVALID_SOCKET };

  server.connect_player_pair(sServer);
  server.keep_deliver_request(sServer);
  getchar();
  //  closesocket(sListen);
  //  closesocket(sServer);
  //  WSACleanup();
}