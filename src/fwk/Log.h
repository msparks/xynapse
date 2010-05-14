/** \file Log.h
 * Logging class.
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

/* Log is a singleton */
class Log;
static Fwk::Ptr<Log> rootLog = NULL;

static const char *levelNames[] = {
  "debug",
  "info",
  "warning",
  "error",
  "critical"
};

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

  static Log::Ptr logNew(const string& loggerName="root") {
    if (rootLog.ptr() == NULL)
      rootLog = new Log("root");
    if (loggerName == "root")
      return rootLog;

    return new Log(loggerName);
  }

  Level debug() { return debug_; }
  Level info() { return info_; }
  Level warning() { return warning_; }
  Level error() { return error_; }
  Level critical() { return critical_; }

  string levelName(Level level) { return levelNames[level]; }

  inline Level level() const { return logLevel_; }
  void levelIs(Level level) {
    if (name() == "root")
      logLevel_ = level;
    else
      rootLog->levelIs(level);
  }

  inline string name() const { return loggerName_; }
  void nameIs(const string& name) { loggerName_ = name; }

  virtual void entryNew(Level level, Fwk::NamedInterface *entity,
                        string funcName, string cond) {
    if (level < rootLog->level())
      return;

    std::cout << timestamp()
              << " [" << levelName(level) << "] " << name() << ": ";
    if (entity != NULL && funcName.size() > 0)
      std::cout << entity->name() << "::" << funcName << ": ";
    else if (funcName.size() > 0)
      std::cout << funcName << ": ";
    std::cout << cond << std::endl;
  }

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
  virtual ~Log() { }
  Log(const string& loggerName)
    : loggerName_(loggerName), logLevel_(info_) { }

  string timestamp() {
    char buf[50];
    time_t rawtime;
    struct tm timeinfo;

    time(&rawtime);
    localtime_r(&rawtime, &timeinfo);

    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return string(buf);
  }

  string loggerName_;
  Level logLevel_;
};

}

#endif