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


static const unsigned short kDefaultTcpPort = 9889;


class Port : public Fwk::Ordinal<Port, unsigned short> {
public:
  Port(unsigned short num) : Fwk::Ordinal<Port, unsigned short>(num) { }
};


class TcpClient : public Fwk::PtrInterface<TcpClient> {
public:
  typedef Fwk::Ptr<TcpClient const> PtrConst;
  typedef Fwk::Ptr<TcpClient> Ptr;

  static TcpClient::Ptr tcpClientNew(int fd, struct sockaddr_in addr) {
    TcpClient *tc = new TcpClient(fd, addr);
    return tc;
  }

  int socket() const { return socket_; }
  struct sockaddr_in address() const { return addr_; }

protected:
  TcpClient(int fd, struct sockaddr_in addr)
    : socket_(fd), addr_(addr) { }

  int socket_;
  struct sockaddr_in addr_;
};


class TcpInterface : public Fwk::PtrInterface<TcpInterface> {
public:
  typedef Fwk::Ptr<TcpInterface> Ptr;
  typedef Fwk::Ptr<TcpInterface const> PtrConst;

  enum Isolation {
    open_,
    closed_
  };

  static TcpInterface::Ptr tcpInterfaceNew() {
    TcpInterface *ti = new TcpInterface();
    return ti;
  }

  void portIs(Port port);
  Port port() const { return port_; }

  void isolationIs(Isolation iso);
  Isolation isolation() const { return isolation_; }

  class Notifiee : public Fwk::PtrInterface<Notifiee> {
  public:
    virtual void notifierIs(TcpInterface::Ptr notifier) {
      if (notifier_ != notifier) {
        notifier_ = notifier;
        notifier->notifieeIs(this);
      }
    }

    TcpInterface::Ptr notifier() const { return notifier_; }

    virtual void onPort() { }
    virtual void onIsolation() { }
    virtual void onMessage(TcpClient::Ptr client,
                           const char *msg, size_t len) { }

  protected:
    TcpInterface::Ptr notifier_;
    Notifiee(TcpInterface::Ptr notifier) : notifier_(notifier) {
      notifier_->notifieeIs(this);
    }
  };

protected:
  void notifieeIs(Notifiee *notifiee) { notifiee_ = notifiee; }

  void acceptThreadFunc();
  void receiveThreadFunc(TcpClient::Ptr client);
  void start();
  void stop();

  TcpInterface()
    : notifiee_(NULL), port_(kDefaultTcpPort), isolation_(closed_),
      running_(false), sock_(0), clientCount_(0),
      log_(Fwk::Log::logNew("TcpInterface")) { }
  ~TcpInterface() { stop(); }

  Notifiee *notifiee_;
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