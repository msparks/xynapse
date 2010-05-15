#ifndef __PYTHONINTERPRETER_H__
#define __PYTHONINTERPRETER_H__

#include <Python.h>

#include <iostream>
#include <map>
#include <string>
#include "fwk/Exception.h"
#include "fwk/PtrInterface.h"

using std::cout;
using std::endl;


class PythonScopedGIL {
public:
  PythonScopedGIL() : gstate_(PyGILState_Ensure()) { }
  ~PythonScopedGIL() { PyGILState_Release(gstate_); }

protected:
  PyGILState_STATE gstate_;
};


class PythonTuple;

class PythonObject : public Fwk::PtrInterface<PythonObject> {
public:
  typedef Fwk::Ptr<PythonObject> Ptr;
  typedef Fwk::Ptr<PythonObject const> PtrConst;

  inline PyObject *ptr() { return object_; }

  static Ptr pythonObjectNew(PyObject *object) {
    PythonScopedGIL l;
    PythonObject::Ptr po = new PythonObject(object);
    Py_XDECREF(object);
    return po;
  }

  inline const Fwk::PtrInterface<PythonObject> *newRef() const {
    PythonScopedGIL l;
    Py_XINCREF(object_);
    return Fwk::PtrInterface<PythonObject>::newRef();
  }

  inline void deleteRef() const {
    PythonScopedGIL l;
    Py_XDECREF(object_);
    return Fwk::PtrInterface<PythonObject>::deleteRef();
  }

  PythonObject::Ptr attribute(const string& attrName) {
    PythonScopedGIL l;
    PyObject *attr = PyObject_GetAttrString(object_, attrName.c_str());
    return PythonObject::pythonObjectNew(attr);
  }

  inline bool callable() const {
    PythonScopedGIL l;
    return PyCallable_Check(object_);
  }

  Ptr operator()(Fwk::Ptr<PythonTuple>& args);
  Ptr operator()();

  Ptr operator[](const string& attrName) { return attribute(attrName); }

protected:
  PythonObject(PyObject *object) : object_(object) { }

  PyObject *object_;
};


class PythonException : public Fwk::Exception {
public:
  PythonException();
};


class PythonModule : public PythonObject {
public:
  typedef Fwk::Ptr<PythonModule> Ptr;
  typedef Fwk::Ptr<PythonModule const> PtrConst;

  static PythonModule::Ptr pythonModuleNew(const string& moduleName) {
    PythonScopedGIL l;
    PyObject *object = PyImport_ImportModule(moduleName.c_str());
    if (object == NULL)
      throw PythonException();
    Ptr m = new PythonModule(object);
    Py_XDECREF(m->ptr());
    return m;
  }

protected:
  PythonModule(PyObject *object) : PythonObject(object) { }
};


class PythonInt : public PythonObject {
public:
  typedef Fwk::Ptr<PythonInt> Ptr;
  typedef Fwk::Ptr<PythonInt const> PtrConst;

  static PythonInt::Ptr pythonIntNew(long val) {
    PythonScopedGIL l;
    PyObject *object = PyInt_FromLong(val);
    if (object == NULL)
      throw PythonException();
    Ptr i = new PythonInt(object);
    Py_XDECREF(i->ptr());
    return i;
  }

protected:
  PythonInt(PyObject *object) : PythonObject(object) { }
};


class PythonString : public PythonObject {
public:
  typedef Fwk::Ptr<PythonString> Ptr;
  typedef Fwk::Ptr<PythonString const> PtrConst;

  static Ptr pythonStringNew(const string& str) {
    PythonScopedGIL l;
    PyObject *object = PyString_FromString(str.c_str());
    if (object == NULL)
      throw PythonException();
    Ptr s = new PythonString(object);
    Py_XDECREF(s->ptr());
    return s;
  }

  static Ptr pythonStringNew(PyObject *object) {
    PythonScopedGIL l;
    if (object == NULL || !PyString_Check(object))
      return pythonStringNew("");
    Ptr s = new PythonString(object);
    Py_XDECREF(s->ptr());
    return s;
  }

  static Ptr pythonStringNew(PythonObject::Ptr o) {
    if (o->ptr() == NULL || !PyString_Check(o->ptr()))
      return pythonStringNew("");
    return new PythonString(o->ptr());
  }

  operator string() const {
    PythonScopedGIL l;
    return string(PyString_AsString(object_));
  }

protected:
  PythonString(PyObject *object) : PythonObject(object) { }
};


class PythonTuple : public PythonObject {
public:
  typedef Fwk::Ptr<PythonTuple> Ptr;
  typedef Fwk::Ptr<PythonTuple const> PtrConst;

  static Ptr pythonTupleNew(ssize_t len) {
    PythonScopedGIL l;
    PyObject *object = PyTuple_New(len);
    if (object == NULL)
      throw PythonException();
    Ptr t = new PythonTuple(object);
    Py_XDECREF(t->ptr());
    return t;
  }

  ssize_t size() const {
    PythonScopedGIL l;
    return PyTuple_Size(object_);
  }

  void itemIs(ssize_t index, PythonObject::Ptr object) {
    PythonScopedGIL l;
    Py_XINCREF(object->ptr());
    PyTuple_SetItem(object_, index, object->ptr());
  }

  PythonObject::Ptr item(ssize_t index) const {
    PythonScopedGIL l;
    PyObject *obj = PyTuple_GetItem(object_, index);
    return PythonObject::pythonObjectNew(obj);
  }

protected:
  PythonTuple(PyObject *object) : PythonObject(object) { }
};


class PythonInterpreter : public Fwk::NamedInterface {
public:
  typedef Fwk::Ptr<PythonInterpreter> Ptr;
  typedef Fwk::Ptr<PythonInterpreter const> PtrConst;

  static Ptr pythonInterpreterNew(const string& progName) {
    return new PythonInterpreter(progName);
  }

  PythonModule::Ptr module(const string& moduleName) const {
    if (modules_.count(moduleName) == 0)
      return PythonModule::pythonModuleNew(NULL);

    return modules_.find(moduleName)->second;
  }

  void moduleIs(const string& moduleName);

protected:
  PythonInterpreter(const string& progName);
  ~PythonInterpreter();

  typedef std::map<std::string, PythonModule::Ptr> ModulesCollectionType;

  ModulesCollectionType modules_;
  PyThreadState *mainThread_;
};

#endif