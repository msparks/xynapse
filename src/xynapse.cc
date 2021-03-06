#include <Python.h>

#include <iostream>
#include <string>
#include <csignal>
#include <map>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/foreach.hpp>

#include "fwk/Exception.h"
#include "fwk/Log.h"
#include "fwk/Ptr.h"
#include "interfaces/TcpInterface.h"
#include "Json.h"
#include "MessageHandler.h"
#include "PythonInterpreter.h"

#define LOG(msg) { \
  std::stringstream ss; \
  ss << msg; \
  log_->entryNew(log_->debug(), ss.str()); \
}

using namespace std;
namespace po = boost::program_options;

static const unsigned short kDefaultTcpPort = 9889;

static PythonInterpreter::Ptr py;
static MessageHandler::Ptr mh;
static Fwk::Log::Ptr log_;


class CommReactor : public CommInterface::Notifiee {
public:
  typedef Fwk::Ptr<CommReactor> Ptr;
  typedef Fwk::Ptr<CommReactor const> PtrConst;

  static Ptr commReactorNew(CommInterface::Ptr notifier) {
    return new CommReactor(notifier);
  }

  void onMessage(CommClient::Ptr client, const string& msg) {
    mh->messageNew(client, msg);
  }

protected:
  CommReactor(CommInterface::Ptr notifier)
    : CommInterface::Notifiee(notifier),
      log_(Fwk::Log::logNew("CommReactor")) { }
  Fwk::Log::Ptr log_;
};


static void
showUsageAndExit(const char *progName, const po::options_description& desc,
                 int exitCode)
{
  cout << "usage: " << progName << " [options]\n";
  cout << desc;

  exit(exitCode);
}


static po::variables_map
parseOptions(int argc, char **argv)
{
  po::options_description desc("options");
  desc.add_options()
    ("help,h", "this help message")
    ("config,c",
     po::value<string>(),
     "configuration file")
  ;

  po::variables_map vm;

  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
  } catch (std::exception& e) {
    cout << "error: " << e.what() << endl;
    showUsageAndExit(argv[0], desc, 1);
  }

  po::notify(vm);

  if (vm.count("help"))
    showUsageAndExit(argv[0], desc, 0);

  return vm;
}


static void
parseConfigFile(const string& fileName, boost::property_tree::ptree& pt)
{
  try {
    boost::property_tree::ini_parser::read_ini(fileName, pt);
  } catch (boost::property_tree::ini_parser::ini_parser_error& e) {
    log_->entryNew(log_->critical(),
                   string("failed to parse config file: ") + e.what());
    exit(1);
  }
}


void
signalHandler(int signo)
{
  if (signo == SIGTERM || signo == SIGHUP || signo == SIGINT) {
    cout << "caught signal, exiting." << endl;
    exit(0);
  }
}


static void
registerMessageHandler(PythonInterpreter::Ptr& py, MessageHandler::Ptr& mh)
{
  /* this tells the Python module how to talk to the MessageHandler */
  PythonObject::Ptr registerFunc;
  registerFunc = py->module("xynapse")->attribute("_register");

  PythonTuple::Ptr args = PythonTuple::pythonTupleNew(1);
  args->itemIs(0, PythonInt::pythonIntNew((long)mh.ptr()));
  (*registerFunc)(args);
}


int
main(int argc, char **argv)
{
  signal(SIGTERM, signalHandler);
  signal(SIGHUP, signalHandler);
  signal(SIGINT, signalHandler);

  log_ = Fwk::Log::logNew();
  log_->levelIs(log_->debug());

  po::variables_map options = parseOptions(argc, argv);
  string configFileName;

  if (options.count("config"))
    configFileName = options["config"].as<string>();
  else
    configFileName = "xynapse.conf";

  try {
    py = PythonInterpreter::pythonInterpreterNew(argv[0]);
    py->moduleIs("xynapse");

    Json::Ptr json = Json::jsonNew(py);
    mh = MessageHandler::messageHandlerNew(json);

    registerMessageHandler(py, mh);
  } catch (Fwk::Exception& e) {
    log_->entryNew(log_->critical(), e);
    exit(1);
  }

  boost::property_tree::ptree config;
  parseConfigFile(configFileName, config);

  BOOST_FOREACH(boost::property_tree::ptree::value_type &v,
                config.get_child("modules")) {
    try {
      py->moduleIs(v.second.data());
      log_->entryNew("loaded module " + v.second.data());
    } catch (Fwk::Exception& e) {
      log_->entryNew(log_->warning(), "failed to load module " + v.second.data());
      log_->entryNew(log_->debug(), e);
    }
  }

  Port port = config.get("interfaces::tcp.port", kDefaultTcpPort);
  CommInterface::Ptr comm = TcpInterface::tcpInterfaceNew(port);
  CommReactor::Ptr reactor = CommReactor::commReactorNew(comm);

  try {
    comm->isolationIs(CommInterface::open_);
  } catch (Fwk::Exception& e) {
    log_->entryNew(log_->critical(), e);
    exit(1);
  }

  while (true)
    sleep(60);

  return 0;
}