#pragma once

#include "../GameCore/GameCore.h"
#include <type_traits>
#include <memory>
#include <iostream>
#include <cstring>
#if _WIN32
  #include <winsock2.h>
  #pragma comment(lib, "ws2_32.lib")
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <unistd.h>
  #include <netinet/in.h>
  using SOCKET = int;
  constexpr SOCKET INVALID_SOCKET = -1;
  constexpr int SOCKET_ERROR = -1;
  #define closesocket close
#endif

#define MAX_REQUEST_SIZE 1024
#define SOCKET_ACT_OK(ret /* int */) ((ret) != SOCKET_ERROR && (ret) != 0)

typedef enum
{
  UNKNOWN_REQUEST,
  START_GAME_REQUEST,
  PLACE_REQUEST,
  MOVE_REQUEST,
  PASS_REQUEST,
  RETRACT_REQUEST,
  RETRACT_ACK_REQUEST,
  HEARTBEAT_REQUEST,

  REQUEST_NUM
} RequestType;

typedef enum
{
  UNKNOWN_SOURCE,
  SERVER_SOURCE,
  OFFEN_PLAYER_SOURCE,
  DEFEN_PLAYER_SOURCE,

  SOURCE_NUM
} SourceType;

struct Request
{
  const RequestType type_;
  const size_t size_;
  SourceType source_;
  Request(const RequestType& type, size_t size, SourceType source) :
    type_(type), size_(size), source_(source) {}
  friend std::ostream& operator<<(std::ostream &strm, const Request& req)
  {
    return strm << "{ type: " << req.type_ << ", size: " << req.size_ << ", source: " << req.source_ << " }";
  }
};

struct StartGameRequest : public Request
{
  bool is_offen_;
  StartGameRequest(bool is_offen, SourceType source = UNKNOWN_SOURCE) :
    is_offen_(is_offen), Request(START_GAME_REQUEST, sizeof(StartGameRequest), source) {}
};

struct PlaceRequest : public Request
{
  AreaType edge_type_;
  Coordinate pos_;
  PlaceRequest(const AreaType& edge_type, const Coordinate& pos, SourceType source = UNKNOWN_SOURCE) :
    Request(PLACE_REQUEST, sizeof(PlaceRequest), source), edge_type_(edge_type), pos_(pos) {}
};

struct MoveRequest : public Request
{
  AreaType old_edge_type_;
  Coordinate old_pos_;
  AreaType new_edge_type_;
  Coordinate new_pos_;
  MoveRequest(const AreaType old_edge_type, const Coordinate& old_pos, const AreaType& new_edge_type, const Coordinate& new_pos, SourceType source = UNKNOWN_SOURCE) :
    Request(MOVE_REQUEST, sizeof(MoveRequest), source), old_edge_type_(old_edge_type), old_pos_(old_pos), new_edge_type_(new_edge_type), new_pos_(new_pos) {}
};

struct PassRequest : public Request
{
  PassRequest(SourceType source = UNKNOWN_SOURCE) : Request(PASS_REQUEST, sizeof(PassRequest), source) {}
};

struct RetractRequest : public Request
{
  RetractRequest(SourceType source = UNKNOWN_SOURCE) : Request(RETRACT_REQUEST, sizeof(RetractRequest), source) {}
};

struct RetractAckRequest : public Request
{
  bool ack_;
  RetractAckRequest(bool ack, SourceType source = UNKNOWN_SOURCE) : Request(RETRACT_ACK_REQUEST, sizeof(RetractAckRequest), source), ack_(ack) {}
};

struct HeartbeatRequest : public Request
{
  HeartbeatRequest(SourceType source = UNKNOWN_SOURCE) : Request(HEARTBEAT_REQUEST, sizeof(HeartbeatRequest), source) {}
};

template <typename R>
void send_request(R&& request, const SOCKET socket)
{
  static_assert(std::is_base_of_v<Request, R>);
  std::cout << "Sending...";
  int ret = send(socket, reinterpret_cast<const char*>(&request), sizeof(request), 0);
  if (!SOCKET_ACT_OK(ret))
  {
    throw std::runtime_error("Send request failed.");
  }
  std::cout << request << std::endl;
}

void send_heartbeat(const SOCKET socket, SourceType source)
{
  return send_request(HeartbeatRequest(source), socket);
}

const std::pair<Request*, int> receive_request(const SOCKET socket, char* buffer)
{
  std::cout << "Receiving...";
  memset(buffer, 0, MAX_REQUEST_SIZE);
  int ret = recv(socket, buffer, MAX_REQUEST_SIZE, 0);
  if (!SOCKET_ACT_OK(ret))
  {
    throw std::runtime_error("Receive request failed.");
  }
  Request* request = reinterpret_cast<Request* const>(buffer);
  std::cout << "[" << ret << "]" << *request << std::endl;
  return std::pair<Request*, int>(request, ret);
}

#if _WIN32
void start_up_winsock(WSADATA& wsaData)
{
  /* Use Winsock DLL v2.2 */
  WORD wVersionRequested = MAKEWORD(2, 2);
  if (WSAStartup(wVersionRequested, &wsaData) != 0)
  {
    throw std::runtime_error("WSAStartup() failed!");
  }
  if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
  {
    WSACleanup();
    throw std::runtime_error("Invalid WinSock version!");
  }
}
#endif

SOCKET create_socket_with_tcp()
{
  SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
  if (s == INVALID_SOCKET)
  {
#if _WIN32
    WSACleanup();
#endif
    throw std::runtime_error("socket() failed!");
  }
  return s;
}

