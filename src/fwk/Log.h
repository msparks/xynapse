/** \file Log.h
 * Logging classes.
 * \author Matt Sparks
 */
#ifndef __FWK__LOG_H_
#define __FWK__LOG_H_

#include <string>
#include <iostream>
#include <ctime>

#include "fwk/Exception.h"
#include "fwk/NamedInterface.h"
#include "fwk/Ptr.h"
#include "fwk/PtrInterface.h"

namespace Fwk {

class Log : public Fwk::PtrInterface<Log> {
public:
  typedef Fwk::Ptr<Log const> PtrConst;
  typedef Fwk::Ptr<Log> Ptr;

  enum Level {
    debug_,
    info_,
    warning_,
    error_,
    critical_
  };

  static Log::Ptr logNew() { return new Log(); }

  Level debug() { return debug_; }
  Level info() { return info_; }
  Level warning() { return warning_; }
  Level error() { return error_; }
  Level critical() { return critical_; }

  string levelName(Level level) {
    switch (level) {
    case debug_:
      return "debug";
      break;
    case info_:
      return "info";
      break;
    case warning_:
      return "warning";
      break;
    case error_:
      return "error";
      break;
    case critical_:
    default:
      return "critical";
      break;
    }
  }

  virtual void entryNew(Level level, Fwk::NamedInterface *entity,
                        string funcName, string cond) { }

  virtual void entryNew(Fwk::NamedInterface *entity,
                        string funcName, string cond) {
    entryNew(info_, entity, funcName, cond);
  }

  virtual void entryNew(string funcName, string cond) {
    entryNew(info_, NULL, funcName, cond);
  }

  virtual void entryNew(Level level, string cond) {
    entryNew(level, NULL, "", cond);
  }

  virtual void entryNew(string cond) {
    entryNew(info_, NULL, "", cond);
  }

  virtual void entryNew(Level level, Exception& e) {
    entryNew(level, e.entity(), e.funcName(), e.message());
  }

  virtual void entryNew(Exception& e) {
    entryNew(critical_, e);
  }

protected:
  virtual ~Log() {}
  Log() {}
};

class StdLog : public Log {
public:
  static Log::Ptr logNew() { return new StdLog(); }

  virtual void entryNew(Level level, Fwk::NamedInterface *entity,
                        string funcName, string cond) {
    std::cout << timestamp() << " [" << levelName(level) << "] ";
    if (entity != NULL && funcName.size() > 0)
      std::cout << entity->name() << "::" << funcName << ": ";
    else if (funcName.size() > 0)
      std::cout << funcName << ": ";
    std::cout << cond << std::endl;
  }

protected:
  virtual ~StdLog() { }
  StdLog() { }

  string timestamp() {
    char buf[50];
    time_t rawtime;
    struct tm timeinfo;

    time(&rawtime);
    localtime_r(&rawtime, &timeinfo);

    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return string(buf);
  }
};

}

#endif
