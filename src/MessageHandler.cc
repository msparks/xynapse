#include "MessageHandler.h"
#include <iostream>
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>


static const unsigned int kWorkers = 5;


void
MessageHandler::handlerNew(const string& protocol, const string& eventName,
                           PythonObject::Ptr& handlerFunc)
{
  Handler h(protocol, eventName, handlerFunc);
  handlers_.push_back(h);

  std::stringstream ss;
  ss << "handler registered: " << "(" << protocol << ", " << eventName + ")";
  log_->entryNew(log_->debug(), ss.str());
}


void
MessageHandler::messageNew(CommClient::Ptr client, const string& msg)
{
  Message m(client, msg);
  messageQueue_->pushBack(m);
}


string
MessageHandler::call(PythonObject::Ptr handlerFunc, const string& msg)
{
  PythonScopedGIL lock;

  PythonObject::Ptr obj = json_->loads(msg);
  PythonTuple::Ptr args = PythonTuple::pythonTupleNew(1);
  args->itemIs(0, obj);

  PythonObject::Ptr ret = (*handlerFunc)(args);
  return json_->dumps(ret);
}


void
MessageHandler::workerThreadFunc(unsigned int workerIndex)
{
  while (running_) {
    Message m = messageQueue_->popFront();
    std::stringstream ss(std::stringstream::in | std::stringstream::out);
    ss << m.msg;
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

    /* TODO: handlers are processed serially in this thread. There should be
       a separate queue of <Handler, Message> tuples that multiple threads
       can process concurrently. */

    /* find handler(s) for this event */
    std::vector<Handler>::iterator it;
    for (it = handlers_.begin(); it != handlers_.end(); ++it) {
      if (it->protocol == protocol && it->eventName == eventName) {
        string response = call(it->handlerFunc, m.msg);

        if (response.size() > 0)
          m.client->messageNew(response.c_str(), response.size());
      }
    }
  }
}


MessageHandler::MessageHandler(PythonInterpreter::Ptr py)
  : py_(py),
    messageQueue_(Fwk::ConcurrentDeque<Message>::concurrentDequeNew()),
    json_(Json::jsonNew(py))
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