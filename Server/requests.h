#pragma once

#include "../GameCore/GameCore.h"
//#include <winsock2.h>
#include <type_traits>
#include <memory>
#define MAX_REQUEST_SIZE 1024
#define SOCKET_ACT_OK(ret /* int */) ((ret) != SOCKET_ERROR && (ret) != 0)

typedef enum
{
  READY_REQUEST,
  START_GAME_REQUEST,
  PLACE_REQUEST,
  MOVE_REQUEST,
  PASS_REQUEST,
  RETRACT_REQUEST,
  RETRACT_ACK_REQUEST,
  HEARTBEAT_REQUEST,

  REQUEST_NUM
} RequestType;

struct Request
{
  RequestType type_;
  Request(const RequestType& type) : type_(type) {}
};

struct ReadyRequest : public Request
{
  ReadyRequest() : Request(READY_REQUEST) {}
};

struct StartGameRequest : public Request
{
  bool is_offen_;
  StartGameRequest(bool is_offen) : is_offen_(is_offen), Request(START_GAME_REQUEST) {}
};

struct PlaceRequest : public Request
{
  AreaType edge_type_;
  Coordinate pos_;
  PlaceRequest(const AreaType& edge_type, const Coordinate& pos) : 
    Request(PLACE_REQUEST), edge_type_(edge_type), pos_(pos) {}
};

struct MoveRequest : public Request
{
  AreaType old_edge_type_;
  Coordinate old_pos_;
  AreaType new_edge_type_;
  Coordinate new_pos_;
  MoveRequest(const AreaType old_edge_type, const Coordinate& old_pos, const AreaType& new_edge_type, const Coordinate& new_pos) : 
    Request(MOVE_REQUEST), old_edge_type_(old_edge_type), old_pos_(old_pos), new_edge_type_(new_edge_type), new_pos_(new_pos) {}
};

struct PassRequest : public Request
{
  PassRequest() : Request(PASS_REQUEST) {}
};

struct RetractRequest : public Request
{
  RetractRequest() : Request(RETRACT_REQUEST) {}
};

struct RetractAckRequest : public Request
{
  bool ack_;
  RetractAckRequest(bool ack) : Request(RETRACT_ACK_REQUEST), ack_(ack) {}
};

struct HeartbeatRequest : public Request
{
  HeartbeatRequest() : Request(HEARTBEAT_REQUEST) {}
};

template <class R>
void send_request(const R& request, SOCKET& socket)
{
  static_assert(std::is_base_of_v<Request, R>);
  if (!SOCKET_ACT_OK(send(socket, reinterpret_cast<const char*>(&request), sizeof(request), 0)))
    throw std::exception("Send request failed.");
}

inline void send_heartbeat(SOCKET& socket)
{
  return send_request(HeartbeatRequest(), socket);
}


