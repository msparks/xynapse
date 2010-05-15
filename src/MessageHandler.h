#ifndef __MESSAGEHANDLER_H__
#define __MESSAGEHANDLER_H__

#include "PythonInterpreter.h"

#include <string>
#include <vector>
#include <boost/thread.hpp>
#include "fwk/Log.h"
#include "fwk/Ptr.h"
#include "fwk/PtrInterface.h"
#include "fwk/ConcurrentDeque.h"
#include "interfaces/CommInterface.h"
#include "Json.h"


using std::string;


class Message {
public:
  Message(CommClient::Ptr _client, const string& _msg)
    : client(_client), msg(_msg) { }
  Message(const Message& other)
    : client(other.client), msg(other.msg) { }

  CommClient::Ptr client;
  const string msg;
};


class Handler {
public:
  Handler(const string& _protocol, const string& _eventName,
          PythonObject::Ptr _handlerFunc)
    : protocol(_protocol), eventName(_eventName), handlerFunc(_handlerFunc) { }
  Handler(const Handler& other)
    : protocol(other.protocol), eventName(other.eventName),
      handlerFunc(other.handlerFunc) { }

  string protocol;
  string eventName;
  PythonObject::Ptr handlerFunc;
};


class WorkUnit {
public:
  WorkUnit(const Handler& h, const Message& m)
    : handler(h), message(m) { }

  Handler handler;
  Message message;
};


class MessageHandler : public Fwk::PtrInterface<MessageHandler> {
public:
  typedef Fwk::Ptr<MessageHandler> Ptr;
  typedef Fwk::Ptr<MessageHandler const> PtrConst;

  enum HandlerType {
    python_
  };

  static Ptr messageHandlerNew(Json::Ptr json) {
    return new MessageHandler(json);
  }

  void handlerNew(const string& protocol, const string& eventName,
                  PythonObject::Ptr& handlerFunc);

  void messageNew(CommClient::Ptr client, const string& msg);

protected:
  MessageHandler(Json::Ptr json);
  ~MessageHandler();

  void workerThreadFunc(unsigned int workerIndex);

  string call(PythonObject::Ptr handlerFunc, const string& msg);

  std::vector<Handler> handlers_;
  Fwk::ConcurrentDeque<WorkUnit>::Ptr workQueue_;
  boost::thread_group workers_;
  bool running_;
  Fwk::Log::Ptr log_;
  Json::Ptr json_;
};

#endif