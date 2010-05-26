#include "PythonInterpreter.h"
#include "MessageHandler.h"

#include <iostream>

using std::cout;
using std::endl;

//#ifdef __cplusplus
extern "C" {
//#endif

static MessageHandler *mh = NULL;


static PyObject *
xynapse_bind(PyObject *self, PyObject *args)
{
  const char *protocol;
  const char *eventName;
  PyObject *callable;

  if (!PyArg_ParseTuple(args, "ssO", &protocol, &eventName, &callable))
    return NULL;

  PythonObject::Ptr obj = PythonObject::pythonObjectNew(callable);
  Py_XINCREF(callable);
  mh->handlerNew(protocol, eventName, obj);

  Py_RETURN_NONE;
}


static PyObject *
xynapse_system(PyObject *self, PyObject *args)
{
  const char *command;
  int sts;

  if (!PyArg_ParseTuple(args, "s", &command))
    return NULL;
  sts = system(command);
  return Py_BuildValue("i", sts);
}


static PyObject *
xynapse__register(PyObject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "l", &mh))
    return NULL;

  Py_RETURN_NONE;
}


static PyMethodDef xynapseMethods[] = {
  {"system",  xynapse_system, METH_VARARGS,
   "Execute a shell command."},
  {"bind", xynapse_bind, METH_VARARGS,
   "Register a callback function for an event."},
  {"_register", xynapse__register, METH_VARARGS, ""},
  {NULL, NULL, 0, NULL}
};


PyMODINIT_FUNC
initxynapse()
{
  Py_InitModule("xynapse", xynapseMethods);
}

//#ifdef __cplusplus
}
//#endif
