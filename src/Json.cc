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
  PythonObject::Ptr dumps = py_->module("json")->attribute("dumps");

  PythonTuple::Ptr dumpsArgs = PythonTuple::pythonTupleNew(1);
  dumpsArgs->itemIs(0, o);

  return *(PythonString::pythonStringNew((*dumps)(dumpsArgs)));
}


Json::Json(PythonInterpreter::Ptr py)
  : Fwk::NamedInterface("Json"), py_(py)
{
  py->moduleIs("json");
}