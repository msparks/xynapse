#include "PythonInterpreter.h"


PythonObject::Ptr
PythonObject::operator()(PythonTuple::Ptr& args)
{
  if (!callable())
    return NULL;
  PythonScopedGIL l;
  PyObject *ret = PyObject_CallObject(object_, args->ptr());
  if (ret == NULL)
    throw PythonException();
  return PythonObject::pythonObjectNew(ret);
}


PythonObject::Ptr
PythonObject::operator()()
{
  if (!callable())
    return NULL;
  PythonScopedGIL l;
  PyObject *ret = PyObject_CallObject(object_, NULL);
  if (ret == NULL)
    throw PythonException();
  return PythonObject::pythonObjectNew(ret);
}


PythonException::PythonException()
  : Exception(NULL, "", "")
{
  PythonScopedGIL l;
  PyObject *ptype;
  PyObject *pvalue;
  PyObject *ptraceback;

  PyErr_Fetch(&ptype, &pvalue, &ptraceback);
  PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);

  PythonObject::Ptr valueObj = PythonObject::pythonObjectNew(pvalue);
  Py_XDECREF(ptype);
  Py_XDECREF(ptraceback);

  if (valueObj->ptr() == NULL) {
    message_ = "No Python exception";
    return;
  }

  PythonObject::Ptr nameObj = (*(*valueObj)["__class__"])["__name__"];
  PythonString::Ptr nameStr = PythonString::pythonStringNew(nameObj->ptr());
  Py_XINCREF(nameObj->ptr());

  PyObject *ret = PyObject_CallMethod(pvalue, (char *)"__str__", NULL);
  PythonString::Ptr valueStr = PythonString::pythonStringNew(ret);

  string name = string(*nameStr);
  string value = string(*valueStr);

  if (value.size() > 0)
    message_ = name + " Exception: " + value;
  else
    message_ = name + " Exception";
}


void
PythonInterpreter::moduleIs(const string& moduleName)
{
  if (modules_.count(moduleName) > 0)
    return;

  PythonModule::Ptr module = PythonModule::pythonModuleNew(moduleName);
  modules_[moduleName] = module;
}


PythonInterpreter::PythonInterpreter(const string& progName)
  : Fwk::NamedInterface("PythonInterpreter")
{
  Py_SetProgramName((char *)progName.c_str());
  Py_InitializeEx(1);
  PyEval_InitThreads();

  PythonModule::Ptr sys = PythonModule::pythonModuleNew("sys");
  PythonString::Ptr dot = PythonString::pythonStringNew(".");
  PyList_Insert((*sys)["path"]->ptr(), 0, dot->ptr());

  PyGILState_Release(PyGILState_UNLOCKED);
}


PythonInterpreter::~PythonInterpreter()
{
  /* clear references to modules */
  modules_.erase(modules_.begin(), modules_.end());

  PyGILState_Ensure();
  Py_Finalize();
}