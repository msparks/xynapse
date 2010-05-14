/** \file PtrInterface.h
 * Smart pointer interface class template.
 * \author David R. Cheriton
 */
#ifndef __FWK__PTRINTERFACE_H_
#define __FWK__PTRINTERFACE_H_

#include "Ptr.h"

namespace Fwk {

template <class T>
class PtrInterface {
public:
  PtrInterface() : ref_(0) {}
  unsigned long references() const { return ref_; }
  inline const PtrInterface * newRef() const { ++ref_; return this; }
  inline void deleteRef() const { if (--ref_ == 0) onZeroReferences(); }

protected:
  virtual ~PtrInterface() {}
  virtual void onZeroReferences() const { delete this; }

private:
  mutable long unsigned ref_;
};

}

#endif
