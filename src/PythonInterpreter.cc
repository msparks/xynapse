#include "PythonInterpreter.h"


PythonObject::Ptr
PythonObject::operator()(PythonTuple::Ptr& args) {
  if (!callable())
    return NULL;
  PythonScopedGIL l;
  PyObject *ret = PyObject_CallObject(object_, args->ptr());
  return PythonObject::pythonObjectNew(ret);
}


PythonObject::Ptr
PythonObject::operator()() {
  if (!callable())
    return NULL;
  PythonScopedGIL l;
  PyObject *ret = PyObject_CallObject(object_, NULL);
  return PythonObject::pythonObjectNew(ret);
}


PythonInterpreter::PythonInterpreter(const string& progName)
  : Fwk::NamedInterface("PythonInterpreter")
{
  Py_SetProgramName((char *)progName.c_str());
  Py_InitializeEx(1);
  PyEval_InitThreads();

  std::string path = Py_GetPath();
  if (strcmp(Py_GetPlatform(), "win") == 0)
    path = ".;" + path;
  else
    path = ".:" + path;

  PySys_SetPath((char *)path.c_str());

  PyGILState_Release(PyGILState_UNLOCKED);
}


PythonInterpreter::~PythonInterpreter()
{
  /* clear references to modules */
  modules_.erase(modules_.begin(), modules_.end());

  PyGILState_Ensure();
  Py_Finalize();
}
