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
#include "PythonInterpreter.h"
#include "MessageHandler.h"


using namespace std;
namespace po = boost::program_options;

static PythonInterpreter::Ptr py;
static MessageHandler::Ptr mh;


class CommReactor : public CommInterface::Notifiee {
public:
  typedef Fwk::Ptr<CommReactor> Ptr;
  typedef Fwk::Ptr<CommReactor const> PtrConst;

  static Ptr commReactorNew(CommInterface::Ptr notifier) {
    return new CommReactor(notifier);
  }

  void onMessage(CommClient::Ptr client, const char *msg, size_t len) {
    mh->messageIs(msg);
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
    cout << "failed to parse config file: " << e.what() << endl;
  }
}


void signalHandler(int signo)
{
  if (signo == SIGTERM || signo == SIGHUP || signo == SIGINT) {
    cout << "caught signal, exiting." << endl;
    exit(0);
  }
}


int
main(int argc, char **argv)
{
  signal(SIGTERM, signalHandler);
  signal(SIGHUP, signalHandler);
  signal(SIGINT, signalHandler);

  Fwk::Log::Ptr log = Fwk::Log::logNew();
  log->levelIs(log->debug());

  po::variables_map options = parseOptions(argc, argv);
  string configFileName;

  if (options.count("config"))
    configFileName = options["config"].as<string>();
  else
    configFileName = "xynapse.conf";

  try {
    py = PythonInterpreter::pythonInterpreterNew(argv[0]);
    mh = MessageHandler::messageHandlerNew(py);
  } catch (Fwk::ResourceException& e) {
    log->entryNew(e);
  }

  log->entryNew("reading from config file: " + configFileName);
  boost::property_tree::ptree pt;
  parseConfigFile(configFileName, pt);

  BOOST_FOREACH(boost::property_tree::ptree::value_type &v,
                pt.get_child("modules")) {
    try {
      py->moduleIs(v.second.data());
      log->entryNew("loaded module " + v.second.data());
    } catch (Fwk::Exception& e) {
      log->entryNew(log->warning(), "failed to load module " + v.second.data());
      log->entryNew(log->debug(), e);
    }
  }

  CommInterface::Ptr comm = TcpInterface::tcpInterfaceNew();

  /* register reactor */
  CommReactor::Ptr reactor = CommReactor::commReactorNew(comm);

  comm->isolationIs(CommInterface::open_);

  while (true)
    sleep(60);

  return 0;
}