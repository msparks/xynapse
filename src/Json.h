#ifndef __JSON_H__
#define __JSON_H__

#include "PythonInterpreter.h"

#include <string>
#include "fwk/NamedInterface.h"
#include "fwk/Ptr.h"

using std::string;


class Json : public Fwk::NamedInterface {
public:
  typedef Fwk::Ptr<Json> Ptr;
  typedef Fwk::Ptr<Json const> PtrConst;

  static Ptr jsonNew(PythonInterpreter::Ptr py) { return new Json(py); }

  PythonObject::Ptr loads(const string& msg) const;
  const string dumps(PythonObject::Ptr o) const;

protected:
  Json(PythonInterpreter::Ptr py);

  PythonInterpreter::Ptr py_;
};

#endif