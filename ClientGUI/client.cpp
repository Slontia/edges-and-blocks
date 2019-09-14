#pragma once

// Client.cpp: �������̨Ӧ�ó������ڵ㡣
//

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <iostream>
#include <exception>
#include "client.h"
#include <QDebug>
#pragma comment(lib, "ws2_32.lib")

Client::Client(const std::string& ip, const int& port) : sClient_(init_socket(ip, port)), source_(UNKNOWN_SOURCE)
{
  if (sClient_ == INVALID_SOCKET)
  {
    throw std::exception("Client boot failed.");
  }
}

Client::~Client() {}

SOCKET Client::init_socket(const std::string& ip, const int& port)
{
  WORD wVersionRequested;
  WSADATA wsaData;
  int ret;
  SOCKET sClient; //�����׽���
  struct sockaddr_in saServer; //��ַ��Ϣ
  BOOL fSuccess = TRUE;

  //WinSock��ʼ��
  wVersionRequested = MAKEWORD(2, 2); //ϣ��ʹ�õ�WinSock DLL�İ汾
  if (WSAStartup(wVersionRequested, &wsaData) != 0)
  {
    printf("WSAStartup() failed!\n");
    return INVALID_SOCKET;
  }
  //ȷ��WinSock DLL֧�ְ汾2.2
  if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
  {
    WSACleanup();
    printf("Invalid WinSock version!\n");
    return INVALID_SOCKET;
  }

  //����Socket,ʹ��TCPЭ��
  sClient = socket(AF_INET, SOCK_STREAM, 0);
  if (sClient == INVALID_SOCKET)
  {
    WSACleanup();
    printf("socket() failed!\n");
    return INVALID_SOCKET;
  }

  //������������ַ��Ϣ
  saServer.sin_family = AF_INET; //��ַ����
  saServer.sin_port = htons(port); //ע��ת��Ϊ�������
  saServer.sin_addr.S_un.S_addr = inet_addr(ip.c_str());

  //���ӷ�����
  ret = connect(sClient, (struct sockaddr *)&saServer, sizeof(saServer));
  if (ret == SOCKET_ERROR)
  {
    printf("connect() failed!\n");
    closesocket(sClient); //�ر��׽���
    WSACleanup();
    return INVALID_SOCKET;
  }
  return sClient;
}

bool Client::wait_for_game_start()
{
  while (true)
  {
    std::cout << "1";
    Request *request = receive_request();
#ifdef TWICE
    request = receive_request();
#endif
    if (request->type_ == HEARTBEAT_REQUEST) { send_heartbeat(sClient_, source_); }
    else if (request->type_ != START_GAME_REQUEST) { throw std::exception("Unexpected request from server before game start."); }
    else
    {
      bool is_offen = reinterpret_cast<StartGameRequest*>(request)->is_offen_;
      source_ = is_offen ? OFFEN_PLAYER_SOURCE : DEFEN_PLAYER_SOURCE;
      return is_offen;
    }
  }
}

Request* Client::receive_request()
{
  return ::receive_request(sClient_, request_buffer_).first;
}

void Client::close_socket()
{
  if (sClient_ != INVALID_SOCKET)
  {
    closesocket(sClient_);
    sClient_ = INVALID_SOCKET;
  }
  WSACleanup();
}

ClientWorker::ClientWorker(const std::string& ip, const int& port) : client_(std::make_unique<Client>(ip, port)) {}

ClientWorker::~ClientWorker() {}

void ClientWorker::wait_for_game_start()
{
  try
  {
    const bool& is_offen = client_->wait_for_game_start();
    emit game_started(is_offen);
  } 
  catch (const std::exception& e)
  {
    std::cout << e.what() << std::endl;
  }
}

void ClientWorker::receive_request()
{
  /* TODO: for some unknown reason, a sent request must be received twice.*/
  Request* request = nullptr;
  try { request = client_->receive_request(); }
  catch (...) {}
  emit request_received(request);
}

void ClientWorker::close_socket() { client_->close_socket(); }

ClientAsyncWrapper::ClientAsyncWrapper(const std::string& ip, const int& port) : worker_(new ClientWorker(ip, port))
{
  worker_->moveToThread(&thread_);

  connect(this, SIGNAL(wait_for_game_start()), worker_, SLOT(wait_for_game_start()));
  connect(this, SIGNAL(receive_request()), worker_, SLOT(receive_request()));
  connect(worker_, SIGNAL(game_started(const bool&)), this, SLOT(exec_game_started_callback(const bool&)));
  connect(worker_, SIGNAL(request_received(Request*)), this, SLOT(exec_request_received_callback(Request*)));
  
  //connect(&thread_, SIGNAL(finished()), worker_, SLOT(deleteLater()));
  thread_.start();
}

ClientAsyncWrapper::~ClientAsyncWrapper()
{
  worker_->close_socket();
  thread_.quit();
  thread_.wait();
  delete worker_;
}

/* Does NOT thread safe. */
void ClientAsyncWrapper::wait_for_game_start_async(GameStartedCallback f)
{
  game_started_f = f;
  emit wait_for_game_start();
}

/* Does NOT thread safe. */
void ClientAsyncWrapper::receive_request_async(RequestReceivedCallback f)
{
  req_recv_f = f;
  emit receive_request();
}
