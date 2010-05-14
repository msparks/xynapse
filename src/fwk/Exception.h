/** \file Exception.h
 * Exception classes.
 * \author Matt Sparks
 */
#ifndef __FWK__EXCEPTION_H__
#define __FWK__EXCEPTION_H__

#include <string>
#include <cstring>
#include <cerrno>

#include "fwk/NamedInterface.h"

using std::string;

namespace Fwk {

class Exception {
public:
  Fwk::NamedInterface *entity() const { return entity_; }
  string funcName() const { return funcName_; }
  string message() const { return message_; }

protected:
  Fwk::NamedInterface *entity_;
  string funcName_;
  string message_;

  Exception(const string& funcName, const string& message)
    : entity_(NULL), funcName_(funcName), message_(message) { }

  Exception(Fwk::NamedInterface *entity, const string& funcName,
            const string& message)
    : entity_(entity), funcName_(funcName), message_(message) { }
};


class RangeException : public Exception {
public:
  RangeException(const string& funcName, const string& message)
    : Exception(funcName, message) { }
  RangeException(Fwk::NamedInterface *entity, const string& funcName,
                 const string& message)
    : Exception(entity, funcName, message) { }
};


class ResourceException : public Exception {
public:
  ResourceException(const string& funcName, const string& message)
    : Exception(funcName, message) { }
  ResourceException(Fwk::NamedInterface *entity, const string& funcName,
                    const string& message)
    : Exception(entity, funcName, message) { }
};


class NameInUseException : public ResourceException {
public:
  NameInUseException(const string& funcName, const string& message)
    : ResourceException(funcName, message) { }
  NameInUseException(Fwk::NamedInterface *entity, const string& funcName,
                     const string& message)
    : ResourceException(entity, funcName, message) { }
};

}

#endif
