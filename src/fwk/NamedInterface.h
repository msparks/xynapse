/** \file NamedInterface.h
 * Defines NamedInterface class.
 * \author David R. Cheriton
 */
#ifndef __FWK__NAMEDINTERFACE_H_
#define __FWK__NAMEDINTERFACE_H_

#include <string>

#include "PtrInterface.h"

namespace Fwk {

class NamedInterface : public PtrInterface<NamedInterface>
{
public:
  std::string name() const { return name_; }

protected:
  NamedInterface(const std::string& name) : name_(name) { }

private:
  std::string name_;
};

}

#endif
