#include "interfaces/TcpInterface.h"

#include <iostream>

using std::cout;
using std::endl;

static const unsigned int kBacklog     = 32;
static const unsigned int kRecvBufSize = 65536;
static const unsigned int kReuseAddr   = 1;
static const unsigned int kMaxClients  = 32;


void
TcpInterface::portIs(Port port)
{
  if (port_ == port)
    return;

  port_ = port;

  if (notifiee_ != NULL)
    notifiee_->onPort();
}


void
TcpInterface::isolationIs(Isolation iso)
{
  if (isolation_ == iso)
    return;

  isolation_ = iso;

  if (isolation_ == open_)
    start();
  else if (isolation_ == closed_)
    stop();

  if (notifiee_ != NULL)
    notifiee_->onIsolation();
}


void
TcpInterface::acceptThreadFunc()
{
  int retval = 0;
  int client;
  struct sockaddr_in addr;
  socklen_t addrLen;

  log_->entryNew(log_->info(), "waiting for connections");

  while (running_) {
    addrLen = sizeof(addr);
    client = accept(sock_, (struct sockaddr *)&addr, &addrLen);
    if (retval == -1) {
      log_->entryNew(log_->critical(), "accept", strerror(errno));
      break;
    }

    boost::lock_guard<boost::mutex> lock(clientCountMutex_);
    if (clientCount_ >= kMaxClients) {
      close(client);
      continue;
    }

    clientCount_++;
    TcpClient::Ptr tc = TcpClient::tcpClientNew(client, addr);

    /* start receive thread for this client */
    boost::thread *thr = new boost::thread(&TcpInterface::receiveThreadFunc,
                                           this, tc);
    threadGroup_.add_thread(thr);
  }

  log_->entryNew(log_->info(), "shutting down");
}


void
TcpInterface::receiveThreadFunc(TcpClient::Ptr client)
{
  char buf[kRecvBufSize + 1];
  buf[kRecvBufSize] = 0;
  int bytes;

  while (running_) {
    bytes = recv(client->socket(), buf, kRecvBufSize, 0);
    if (bytes <= 0)
      break;
    buf[bytes] = 0;

    if (notifiee_ != NULL)
      notifiee_->onMessage(client, buf, bytes);
  }

  close(client->socket());

  boost::lock_guard<boost::mutex> lock(clientCountMutex_);
  clientCount_--;
}


void
TcpInterface::start()
{
  sock_ = socket(PF_INET, SOCK_STREAM, 0);
  if (sock_ == -1)
    throw Fwk::ResourceException("socket", strerror(errno));

  setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, &kReuseAddr, sizeof(kReuseAddr));

  /* TODO: be able to configure bind address */
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port_.value());
  inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr.s_addr);

  int ret = bind(sock_, (struct sockaddr *)&addr, sizeof(addr));
  if (ret == -1)
    throw Fwk::ResourceException("bind", strerror(errno));

  ret = listen(sock_, kBacklog);
  if (ret == -1)
    throw Fwk::ResourceException("listen", strerror(ret));

  running_ = true;

  /* start accept thread. */
  boost::thread *thr = new boost::thread(&TcpInterface::acceptThreadFunc, this);
  threadGroup_.add_thread(thr);
}


void
TcpInterface::stop()
{
  running_ = false;
  close(sock_);
  threadGroup_.interrupt_all();
  threadGroup_.join_all();
}
