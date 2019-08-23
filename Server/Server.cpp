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
    std::cout << "Game Start!" << std::endl;
    send_start_game_requests(sServer);
  }

  void keep_deliver_request(SOCKET sServer[2])
  {
    /* TODO: judge offen or defen */
    for (int player_no = 0; ; player_no = 1 - player_no)
    {
      char request_buffer[MAX_REQUEST_SIZE] = {0};
      std::cout << "Waiting for request from player " << player_no << std::endl;
      int size = recv(sServer[player_no], request_buffer, MAX_REQUEST_SIZE, 0);
      std::cout << size << " ";
      size = recv(sServer[player_no], request_buffer, MAX_REQUEST_SIZE, 0);
      std::cout << size << std::endl;
      std::cout << "Received request from player " << player_no << std::endl;
      if (!SOCKET_ACT_OK(size) || !SOCKET_ACT_OK(send(sServer[1 - player_no], request_buffer, size + 1, 0)))
      {
        /* TODO: handle one player disconnect */
        break;
      }
      std::cout << "Sent request to player " << 1 - player_no << std::endl;
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
    sListen_ = socket(AF_INET, SOCK_STREAM, 0);
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

  static void wait_for_new_clients(SOCKET sServer[2], SOCKET& sListen)
  {
    struct sockaddr_in saClient;
    int length = sizeof(saClient);
    for (int i = 0; i < 2; ++i)
    {
      std::cout << "Waiting for new player." << std::endl;
      while (sServer[i] == INVALID_SOCKET)
      {
        sServer[i] = accept(sListen, (struct sockaddr *)&saClient, &length);
      }
      std::cout << "New player joins." << std::endl;
    }
  }

  static void wait_for_heartbeat(SOCKET& socket)
  {
    char request_buffer[MAX_REQUEST_SIZE] = {0};
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
    std::cout << "Now check connections...";
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
        std::cout << "Failed!" << std::endl;
        return false;
      }
      std::cout << "Successed...";
    }
    std::cout << "All Successful!" << std::endl;
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
  //  getchar();
  //  closesocket(sListen);
  //  closesocket(sServer);
  //  WSACleanup();
}