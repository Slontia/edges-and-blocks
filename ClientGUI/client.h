#pragma once

// Client.cpp: �������̨Ӧ�ó������ڵ㡣
//

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include "../Server/requests.h"

#define SERVER_PORT 5208 //�����˿�

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
  template<class R> void send_request(const R& request) { ::send_request(request, sClient_); }
  bool is_offen();
};

