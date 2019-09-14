#pragma once

// Client.cpp: 定义控制台应用程序的入口点。
//

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include "../Server/requests.h"
#include <QThread>
#include <functional>
#include <QDebug>

class Client;
class ClientWorker;

class ClientAsyncWrapper : public QObject
{
  Q_OBJECT

public:
  typedef std::function<void(Request* const)> RequestReceivedCallback;
  typedef std::function<void(const bool&)> GameStartedCallback;
private:
  ClientWorker* worker_;
  QThread thread_;
  GameStartedCallback game_started_f;
  RequestReceivedCallback req_recv_f;
signals:
  void wait_for_game_start();
  void receive_request();
private slots:
  void exec_game_started_callback(const bool& is_offen) { game_started_f(is_offen); }
  void exec_request_received_callback(Request* request) { req_recv_f(request); }
public:
  ClientAsyncWrapper(const std::string& ip, const int& port);
  ~ClientAsyncWrapper();
  void wait_for_game_start_async(GameStartedCallback f);
  void receive_request_async(RequestReceivedCallback f);
  template<class R> void send_request(R& request) { worker_->send_request(request); }
};

class ClientWorker : public QObject
{
  Q_OBJECT

private:
  std::unique_ptr<Client> client_;
signals:
  void game_started(const bool& is_offen);
  void request_received(Request*);
public slots:
  void wait_for_game_start();
  void receive_request();
public:
  ClientWorker(const std::string& ip, const int& port);
  ~ClientWorker();
  template<class R> void send_request(R& request) { qDebug() << "Send"; client_->send_request(request); }
  void close_socket();
};

class Client
{
private:
  SOCKET sClient_;
  char request_buffer_[MAX_REQUEST_SIZE];
  SourceType source_;

public:
  Client(const std::string& ip, const int& port);
  ~Client();

private:
  SOCKET init_socket(const std::string& ip, const int& port);

public:
  bool wait_for_game_start();
  Request* receive_request();
  template<class R> void send_request(R& request) 
  {
    request.source_ = source_;
    ::send_request(request, sClient_); 
  }
  void close_socket();
};

