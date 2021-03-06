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


static const unsigned int kRecvBufSize = 65536;


class Port : public Fwk::Ordinal<Port, unsigned short> {
public:
  Port(unsigned short num) : Fwk::Ordinal<Port, unsigned short>(num) { }
};


class TcpClient : public CommClient {
public:
  static Ptr tcpClientNew(int fd, struct sockaddr_in addr) {
    return new TcpClient(fd, addr);
  }

  virtual string message();
  virtual void messageNew(const string& msg);

protected:
  TcpClient(int fd, struct sockaddr_in addr)
    : extraSize_(0), socket_(fd), addr_(addr) { }
  ~TcpClient() { close(socket_); }

  char buf_[kRecvBufSize + 1];
  ssize_t extraSize_;
  int socket_;
  struct sockaddr_in addr_;
};


class TcpInterface : public CommInterface {
public:
  typedef Fwk::Ptr<TcpInterface> Ptr;
  typedef Fwk::Ptr<TcpInterface const> PtrConst;

  static Ptr tcpInterfaceNew(Port port) { return new TcpInterface(port); }

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