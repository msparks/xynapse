#include "MessageHandler.h"
#include <iostream>
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>


static const unsigned int kWorkers = 5;


void
MessageHandler::handlerIs(const string& protocol, const string& eventName,
                          PythonObject::Ptr& handlerFunc)
{
  struct _pyHandler h = {protocol, eventName, handlerFunc};
  pyHandlers_.push_back(h);

  std::stringstream ss;
  ss << "handler registered: " << "(" << protocol << ", " << eventName + ")";
  log_->entryNew(log_->debug(), ss.str());
}


void
MessageHandler::messageIs(const string& msg)
{
  messageQueue_->pushBack(msg);
}


void
MessageHandler::call(PythonObject::Ptr handlerFunc, const string& msg)
{
  PythonScopedGIL lock;

  PythonObject::Ptr loads = py_->module("json")->attribute("loads");
  PythonString::Ptr str = PythonString::pythonStringNew(msg);

  PythonTuple::Ptr loadsArgs = PythonTuple::pythonTupleNew(1);
  loadsArgs->itemIs(0, str);

  PythonObject::Ptr obj = (*loads)(loadsArgs);

  PythonTuple::Ptr args = PythonTuple::pythonTupleNew(1);
  args->itemIs(0, obj);

  (*handlerFunc)(args);
}


void
MessageHandler::workerThreadFunc(unsigned int workerIndex)
{
  while (running_) {
    string msg = messageQueue_->popFront();
    std::stringstream ss(std::stringstream::in | std::stringstream::out);
    ss << msg;
    boost::property_tree::ptree pt;
    try {
      boost::property_tree::json_parser::read_json(ss, pt);
    } catch (boost::property_tree::json_parser::json_parser_error& e) {
      log_->entryNew(log_->debug(), "unable to parse incoming message");
      return;
    }

    string protocol = pt.get("protocol", "");
    string eventName = pt.get("eventName", "");
    if (protocol.size() == 0 || eventName.size() == 0)
      return;

    /* find handler(s) for this event */
    std::vector<struct _pyHandler>::iterator it;
    for (it = pyHandlers_.begin(); it != pyHandlers_.end(); ++it) {
      if (it->protocol == protocol && it->eventName == eventName)
        call(it->handlerFunc, msg);
    }
  }
}


MessageHandler::MessageHandler(PythonInterpreter::Ptr py)
  : py_(py),
    messageQueue_(Fwk::ConcurrentDeque<string>::concurrentDequeNew())
{
  log_ = Fwk::Log::logNew("MessageHandler");

  py->moduleIs("json");
  py->moduleIs("xynapse");

  /* register MessageHandler object in binding module */
  PythonObject::Ptr registerFunc;
  registerFunc = py->module("xynapse")->attribute("_register");

  PythonTuple::Ptr args = PythonTuple::pythonTupleNew(1);
  args->itemIs(0, PythonInt::pythonIntNew((long)this));
  (*registerFunc)(args);

  /* start worker threads. */
  running_ = true;
  for (unsigned int i = 0; i < kWorkers; ++i) {
    boost::thread *thr = new boost::thread(&MessageHandler::workerThreadFunc,
                                           this, i);
    workers_.add_thread(thr);
  }

  std::stringstream ss;
  ss << "initialized with " << kWorkers << " worker threads";
  log_->entryNew(log_->info(), ss.str());
}


MessageHandler::~MessageHandler()
{
  running_ = false;
  workers_.interrupt_all();
  workers_.join_all();
}