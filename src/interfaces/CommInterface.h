#ifndef __INTERFACES__COMMINTERFACE_H__
#define __INTERFACES__COMMINTERFACE_H__

#include "fwk/Nominal.h"
#include "fwk/Notifiee.h"
#include "fwk/Ptr.h"
#include "fwk/PtrInterface.h"


class CommClient : public Fwk::PtrInterface<CommClient> {
public:
  typedef Fwk::Ptr<CommClient const> PtrConst;
  typedef Fwk::Ptr<CommClient> Ptr;

  virtual ssize_t message(char *buf, size_t bufsize) = 0;
  virtual ssize_t messageNew(const char *buf, size_t len) = 0;

protected:
  virtual ~CommClient() { }
};


class CommInterface : public Fwk::PtrInterface<CommInterface> {
public:
  typedef Fwk::Ptr<CommInterface> Ptr;
  typedef Fwk::Ptr<CommInterface const> PtrConst;

  enum Isolation {
    open_,
    closed_
  };

  virtual void isolationIs(Isolation iso) { isolation_ = iso; }
  virtual Isolation isolation() const { return isolation_; }

  class Notifiee : public Fwk::PtrInterface<Notifiee> {
  public:
    virtual void notifierIs(CommInterface::Ptr notifier) {
      if (notifier_ != notifier) {
        notifier_ = notifier;
        notifier->notifieeIs(this);
      }
    }

    CommInterface::Ptr notifier() const { return notifier_; }

    virtual void onIsolation() { }
    virtual void onMessage(CommClient::Ptr client,
                           const char *msg, const size_t len) { }

  protected:
    CommInterface::Ptr notifier_;
    Notifiee(CommInterface::Ptr notifier) : notifier_(notifier) {
      notifier_->notifieeIs(this);
    }
  };

protected:
  virtual void notifieeIs(Notifiee *notifiee) { notifiee_ = notifiee; }
  CommInterface()
    : notifiee_(NULL), isolation_(closed_) { }
  ~CommInterface() { }

  Notifiee *notifiee_;
  Isolation isolation_;
};

#endif