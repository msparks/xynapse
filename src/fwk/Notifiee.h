/** \file Notifiee.h
 * Notifiee class templates.
 * \author David R. Cheriton
 */
#ifndef __FWK__NOTIFIEE_H__
#define __FWK__NOTIFIEE_H__

#include <string>

#include "Ptr.h"
#include "PtrInterface.h"

namespace Fwk {

class RootNotifiee : public PtrInterface<RootNotifiee> {
  /* Deliberately empty */
};


template <typename Notifier>
class BaseNotifiee : public RootNotifiee {
public:
  BaseNotifiee(Notifier* n = NULL) : notifier_(n) {
    if (n != NULL) {
      n->lastNotifieeIs(static_cast<typename Notifier::Notifiee*>(this));
    }
  }

  ~BaseNotifiee() {
    if (notifier_ != NULL) {
      notifier_->lastNotifieeIs(0);
    }
  }

  Fwk::Ptr<Notifier> notifier() const {
    return notifier_;
  }

  void notifierIs(Fwk::Ptr<Notifier> n) {
    if (notifier_ != n) {
      if (notifier_ != NULL) {
        notifier_->lastNotifieeIs(0);
      }
      notifier_ = n;
      if (n != NULL) {
        n->lastNotifieeIs(static_cast<typename Notifier::Notifiee*>(this));
      }
    }
  }

private:
  Fwk::Ptr<Notifier> notifier_;
};

}

#endif
