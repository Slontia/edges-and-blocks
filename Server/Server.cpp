#include <stdio.h>
#include <signal.h>
#include <exception>
#include <iostream>
#include <cassert>
#include <array>
#include <thread>
#include <random>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>
#include "requests.h"

static const int kDefaultServerPort = 9810;

class Server
{
public:
  Server() : sListen_(init_socket_listener())
  {
    if (sListen_ == INVALID_SOCKET)
    {
      throw std::runtime_error("Server boot failed.");
    }
  }

  std::array<SOCKET, 2> connect_player_pair(const uint64_t game_id)
  {
    std::array<SOCKET, 2> sServer = { INVALID_SOCKET, INVALID_SOCKET };
    do
    {
      wait_for_new_clients(game_id, sServer, sListen_);
    } while (!check_clients_ready(game_id, sServer));
    printf("[game_id:%llu] Game start!\n", game_id);
    send_start_game_requests(sServer);
    return sServer;
  }

  void keep_deliver_request(const uint64_t game_id, std::array<SOCKET, 2> sServer)
  {
    /* TODO: judge offen or defen */
    for (int player_no = 0; ; player_no = 1 - player_no)
    {
      char request_buffer[MAX_REQUEST_SIZE] = { 0 };
      printf("[game_id:%llu] Waiting for request from player %d...\n", game_id, player_no);
      try
      {
        auto recv_ret = ::receive_request(sServer[player_no], request_buffer);
        Request* const& request = recv_ret.first;
        const int& size = recv_ret.second;
        int send_ret = send(sServer[1 - player_no], request_buffer, size, 0);
        if (!SOCKET_ACT_OK(send_ret))
        {
          printf("[game_id:%llu] Send to player %d failed ret:%d\n", game_id, player_no, send_ret);
          break;
        }
      }
      catch (...)
      {
        printf("[game_id:%llu] Exception occurs\n", game_id);
        break;
      }
      printf("[game_id:%llu] Send to player %d\n", game_id, player_no);
    }
    closesocket(sServer[0]);
    closesocket(sServer[1]);
  }

  int Port() const { return port_; }

private:
  SOCKET sListen_;
  int port_;

  static int get_port()
  {
    static constexpr auto kConfigFile =
#if _WIN32
      TEXT("server-port");
#else
      ("server-port");
#endif
    int port = kDefaultServerPort;
    std::fstream file(kConfigFile);
    if (file)
    {
      file >> port;
    }
    std::cout << "Input server port (default: " << port << "): ";
    if (std::string port_s; std::getline(std::cin, port_s), !port_s.empty())
    {
      if (int port_tmp = 0; !(std::stringstream(port_s) >> port_tmp))
      {
        throw std::runtime_error("invalid port.");
      }
      else
      {
        port = port_tmp;
      }
    }
    file << port;
    file.close();
    std::cout << "Init socket listener with port " << port << std::endl;
    return port;
  }

  static SOCKET init_socket_listener()
  {
    try
    {
#if _WIN32
      WSADATA wsaData; // wsa data cannot be released
      ::start_up_winsock(wsaData);
#endif
      SOCKET sListen = ::create_socket_with_tcp();
      bind_socket_listener(sListen, get_port());
      begin_listen(sListen);
      return sListen;
    }
    catch (const std::runtime_error& e)
    {
      std::cout << e.what() << std::endl;
      return INVALID_SOCKET;
    }
  }

  static void bind_socket_listener(const SOCKET sListen, const int& port)
  {
    struct sockaddr_in saServer;
    saServer.sin_family = AF_INET;
    saServer.sin_port = htons(port);
    saServer.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sListen, (struct sockaddr*)&saServer, sizeof(saServer)) == SOCKET_ERROR)
    {
      closesocket(sListen);
#if _WIN32
      WSACleanup();
      throw std::runtime_error("bind() faild! code: " + WSAGetLastError());
#else
      throw std::runtime_error("bind() faild!");
#endif
    }
  }

  static void begin_listen(SOCKET sListen)
  {
    if (listen(sListen, 5) == SOCKET_ERROR)
    {
      closesocket(sListen);
#if _WIN32
      throw std::runtime_error("listen() faild! code: " + WSAGetLastError());
#else
      throw std::runtime_error("listen() faild!");
#endif
    }
  }


  static void wait_for_new_clients(const uint64_t game_id, std::array<SOCKET, 2>& sServer, const SOCKET sListen)
  {
    struct sockaddr_in saClient;
#if _WIN32
    int
#else
    socklen_t
#endif
      length = sizeof(saClient);
    for (int i = 0; i < 2; ++i)
    {
      if (sServer[i] == INVALID_SOCKET)
      {
        printf("[game_id:%llu] Waiting for new player...\n", game_id);
        while (sServer[i] == INVALID_SOCKET)
        {
          sServer[i] = accept(sListen, (struct sockaddr*)&saClient, &length);
        }
        printf("[game_id:%llu] New player %d joins!\n", game_id, i);
      }
    }
  }

  static void wait_for_heartbeat(SOCKET& socket)
  {
    char request_buffer[MAX_REQUEST_SIZE] = { 0 };
    Request* request = ::receive_request(socket, request_buffer).first;
    if (request->type_ != HEARTBEAT_REQUEST)
    {
      throw std::runtime_error("Unexpected non-heartbeat request.");
    }
  }

  static bool check_clients_ready(const uint64_t game_id, std::array<SOCKET, 2>& sServer)
  {
    printf("[game_id:%llu] Now check connections\n", game_id);
    for (int i = 0; i < 2; ++i)
    {
      assert(sServer[i] != INVALID_SOCKET);
      try
      {
        printf("[game_id:%llu] Sending heartbeat to player %d\n", game_id, i);
        send_heartbeat(sServer[i], SERVER_SOURCE);
        wait_for_heartbeat(sServer[i]);
      }
      catch (const std::runtime_error& e)
      {
        std::cerr << e.what() << std::endl;
        closesocket(sServer[i]);
        sServer[i] = INVALID_SOCKET;
        std::cout << "Failed!" << std::endl;
        printf("[game_id:%llu] Check connection with player %d failed\n", game_id, i);
        return false;
      }
      printf("[game_id:%llu] Check connection with player %d successful\n", game_id, i);
    }
    printf("[game_id:%llu] All players ready!\n", game_id);
    return true;
  }

  static void send_start_game_requests(const std::array<SOCKET, 2>& sServer)
  {
    std::srand(std::time(nullptr));
    bool is_offen = std::rand() % 2;
    for (int i = 0; i < 2; ++i)
    {
      /* TODO: check exception. */
      send_request(StartGameRequest(is_offen, SERVER_SOURCE), sServer.at(i));
      is_offen = !is_offen;
    }
  }
};

int main()
{
  Server server;
  std::cout << "Server is running!" << std::endl;

  for (uint64_t game_id = 0; ; ++game_id)
  {
    auto players = server.connect_player_pair(game_id);
    std::thread(&Server::keep_deliver_request, &server, game_id, players).detach();
  }

  return 0;
}
