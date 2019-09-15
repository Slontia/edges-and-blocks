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

#define SERVER_PORT 9810

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
      std::cout << "Waiting for request from player " << player_no << "...";
      try
      {
        auto recv_ret = ::receive_request(sServer[player_no], request_buffer);
        Request* const& request = recv_ret.first;
        const int& size = recv_ret.second;
        int send_ret = send(sServer[1 - player_no], request_buffer, size, 0);
        if (!SOCKET_ACT_OK(send_ret)) { break; }
      }
      catch (...) { break; }
      std::cout << "Sent to player " << 1 - player_no << std::endl;
    }
  }

private:
  SOCKET sListen_;

  static void bind_socket_listener(SOCKET sListen, const int& port)
  {
    struct sockaddr_in saServer;
    saServer.sin_family = AF_INET;
    saServer.sin_port = htons(port);
    saServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    if (bind(sListen, (struct sockaddr *)&saServer, sizeof(saServer)) == SOCKET_ERROR)
    {
      closesocket(sListen);
      WSACleanup();
      throw std::exception("bind() faild! code: " + WSAGetLastError());
    }
  }

  static void begin_listen(SOCKET sListen)
  {
    if (listen(sListen, 5) == SOCKET_ERROR)
    {
      closesocket(sListen);
      throw std::exception("listen() faild! code: " + WSAGetLastError());
    }
  }

  static SOCKET init_socket_listener()
  {
    try
    {
      WSADATA wsaData;
      ::start_up_winsock(wsaData);
      SOCKET sListen = ::create_socket_with_tcp();
      bind_socket_listener(sListen, SERVER_PORT);
      begin_listen(sListen);
      return sListen;
    }
    catch (const std::exception& e)
    {
      std::cout << e.what() << std::endl;
      return INVALID_SOCKET;
    }
  }

  static void wait_for_new_clients(SOCKET sServer[2], SOCKET& sListen)
  {
    struct sockaddr_in saClient;
    int length = sizeof(saClient);
    for (int i = 0; i < 2; ++i)
    {
      if (sServer[i] == INVALID_SOCKET)
      {
        std::cout << "Waiting for new player...";
        while (sServer[i] == INVALID_SOCKET)
        {
          sServer[i] = accept(sListen, (struct sockaddr *)&saClient, &length);
        }
        std::cout << "New player " << i << " joins!" << std::endl;
      }
    }
  }

  static void wait_for_heartbeat(SOCKET& socket)
  {
    char request_buffer[MAX_REQUEST_SIZE] = {0};
    Request *request = ::receive_request(socket, request_buffer).first;
    if (request->type_ != HEARTBEAT_REQUEST)
    {
      throw std::exception("Unexpected non-heartbeat request.");
    }
  }

  static bool check_clients_ready(SOCKET sServer[2], SOCKET& sListen)
  {
    std::cout << "Now check connections..." << std::endl;
    for (int i = 0; i < 2; ++i)
    {
      assert(sServer[i] != INVALID_SOCKET);
      try
      {
        std::cout << " - Sending heartbeat to player " << i << "...";
        send_heartbeat(sServer[i], SERVER_SOURCE);
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
      std::cout << "Successful!" << std::endl;
    }
    std::cout << " - All players ready!" << std::endl;
    return true;
  }

  static void send_start_game_requests(SOCKET sServer[2])
  {
    /* TODO: make is_offen random. */
    bool is_offen = true;
    for (int i = 0; i < 2; ++i)
    {
      /* TODO: check exception. */
      send_request(StartGameRequest(is_offen, SERVER_SOURCE), sServer[i]);
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