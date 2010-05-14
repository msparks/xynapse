#ifndef __MESSAGEHANDLER_H__
#define __MESSAGEHANDLER_H__

#include "PythonInterpreter.h"

#include <string>
#include <vector>
#include "fwk/Log.h"
#include "fwk/Ptr.h"
#include "fwk/PtrInterface.h"
#include "fwk/ConcurrentDeque.h"
#include "interfaces/TcpInterface.h"


using std::string;


class MessageHandler : public Fwk::PtrInterface<MessageHandler> {
public:
  typedef Fwk::Ptr<MessageHandler> Ptr;
  typedef Fwk::Ptr<MessageHandler const> PtrConst;

  enum HandlerType {
    python_
  };

  static Ptr messageHandlerNew(PythonInterpreter::Ptr py) {
    return new MessageHandler(py);
  }

  void handlerIs(const string& protocol, const string& eventName,
                 PythonObject::Ptr& handlerFunc);

  void messageIs(const string& msg);

protected:
  MessageHandler(PythonInterpreter::Ptr py);
  ~MessageHandler();

  void workerThreadFunc(unsigned int workerIndex);

  void call(PythonObject::Ptr handlerFunc, const string& msg);

  struct _pyHandler {
    string protocol;
    string eventName;
    PythonObject::Ptr handlerFunc;
  };

  PythonInterpreter::Ptr py_;
  std::vector<struct _pyHandler> pyHandlers_;
  Fwk::ConcurrentDeque<string>::Ptr messageQueue_;
  boost::thread_group workers_;
  bool running_;
  Fwk::Log::Ptr log_;
};

#endif