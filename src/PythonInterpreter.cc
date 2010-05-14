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


void
PythonInterpreter::moduleIs(const string& moduleName)
{
  if (modules_.count(moduleName) > 0)
    return;

  PythonModule::Ptr module = PythonModule::pythonModuleNew(moduleName);
  if (module->ptr() == NULL) {
    PythonScopedGIL l;
    if (PyErr_Occurred() != NULL) {
      cout << "an error occurred" << endl;
      PyErr_Print();
    } else {
      cout << "module failed to import,  but no exception?" << endl;
    }
    throw Fwk::ResourceException(this, "moduleIs",
                                 "failed to load module " + moduleName);
  }

  modules_[moduleName] = module;
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
