#ifndef __INTERFACES__TCP_HPP__
#define __INTERFACES__TCP_HPP__

#include <arpa/inet.h>
#include <sys/socket.h>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include "fwk/Exception.h"
#include "fwk/Log.h"
#include "fwk/Nominal.h"
#include "fwk/Notifiee.h"
#include "fwk/Ptr.h"
#include "fwk/PtrInterface.h"
#include "interfaces/CommInterface.h"


static const unsigned short kDefaultTcpPort = 9889;


class Port : public Fwk::Ordinal<Port, unsigned short> {
public:
  Port(unsigned short num) : Fwk::Ordinal<Port, unsigned short>(num) { }
};


class TcpClient : public CommClient {
public:
  static Ptr tcpClientNew(int fd, struct sockaddr_in addr) {
    return new TcpClient(fd, addr);
  }

  virtual ssize_t message(char *buf, size_t bufsize) {
    ssize_t bytes = recv(socket_, buf, bufsize, 0);
    if (bytes >= 0)
      buf[bytes] = 0;
    return bytes;
  }

  virtual ssize_t messageIs(const char *buf, size_t len) {
    return send(socket_, buf, len, 0);
  }

protected:
  TcpClient(int fd, struct sockaddr_in addr)
    : socket_(fd), addr_(addr) { }
  ~TcpClient() { close(socket_); }

  int socket_;
  struct sockaddr_in addr_;
};


class TcpInterface : public CommInterface {
public:
  typedef Fwk::Ptr<TcpInterface> Ptr;
  typedef Fwk::Ptr<TcpInterface const> PtrConst;

  static Ptr tcpInterfaceNew(Port port=kDefaultTcpPort) {
    return new TcpInterface(port);
  }

  Port port() const { return port_; }

  void isolationIs(Isolation iso);

protected:
  void acceptThreadFunc();
  void receiveThreadFunc(CommClient::Ptr client);
  void start();
  void stop();

  TcpInterface(Port port)
    : port_(port), isolation_(closed_),
      running_(false), sock_(0), clientCount_(0),
      log_(Fwk::Log::logNew("TcpInterface")) { }
  ~TcpInterface() { stop(); }

  Port port_;
  Isolation isolation_;
  bool running_;
  int sock_;
  boost::thread_group threadGroup_;
  boost::mutex clientCountMutex_;
  unsigned int clientCount_;
  Fwk::Log::Ptr log_;
};

#endif