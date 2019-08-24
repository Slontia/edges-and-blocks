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
  typedef std::function<void(const Request&)> RequestReceivedCallback;
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
  void exec_request_received_callback(const Request& request) { req_recv_f(request); }
public:
  ClientAsyncWrapper();
  ~ClientAsyncWrapper();
  void wait_for_game_start_async(GameStartedCallback f);
  void receive_request_async(RequestReceivedCallback f);
  template<class R> void send_request(const R& request) { 
    qDebug() << "send"; 
    try { worker_->send_request(request); }
    catch (std::exception& e) { qDebug() << e.what(); return; }
    qDebug() << "suc";
  }
};

class ClientWorker : public QObject
{
  Q_OBJECT

private:
  std::unique_ptr<Client> client_;
signals:
  void game_started(const bool& is_offen);
  void request_received(const Request&);
public slots:
  void wait_for_game_start();
  void receive_request();
public:
  ClientWorker();
  ~ClientWorker();
  template<class R> void send_request(const R& request) { qDebug() << "worker send"; client_->send_request(request); }
};

class Client
{
private:
  SOCKET sClient_;
  char request_buffer_[MAX_REQUEST_SIZE];

public:
  Client();
  ~Client();

private:
  SOCKET init_socket();

public:
  bool wait_for_game_start();
  Request& receive_request();
  template<class R> void send_request(const R& request) { ::send_request(request, sClient_); }
  bool is_offen();
};

