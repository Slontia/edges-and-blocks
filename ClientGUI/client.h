#pragma once

// Client.cpp: 定义控制台应用程序的入口点。
//

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include "../Server/requests.h"

#define SERVER_PORT 5208 //侦听端口

class Client
{
private:
  SOCKET sClient_;
  bool is_offen_;
  char request_buffer_[MAX_REQUEST_SIZE];

public:
  Client();
  ~Client();

private:
  SOCKET init_socket();

public:
  void wait_for_game_start();
  Request& receive_request();
  bool is_offen();
};

