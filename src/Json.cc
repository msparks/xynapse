#include "Json.h"


PythonObject::Ptr
Json::loads(const string& msg) const
{
  PythonObject::Ptr loads = py_->module("json")->attribute("loads");
  PythonString::Ptr str = PythonString::pythonStringNew(msg);

  PythonTuple::Ptr loadsArgs = PythonTuple::pythonTupleNew(1);
  loadsArgs->itemIs(0, str);

  return (*loads)(loadsArgs);
}


const string
Json::dumps(PythonObject::Ptr o) const
{
  return "";
}


Json::Json(PythonInterpreter::Ptr py)
  : Fwk::NamedInterface("Json"), py_(py)
{
  py->moduleIs("json");
}