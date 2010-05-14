/** \file ConcurrentDeque.h
 * Thread-safe double-ended queue data structure.
 * \author Matt Sparks
 */
#ifndef __FWK__CONCURRENTDEQUE_H__
#define __FWK__CONCURRENTDEQUE_H__

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>

#include "Deque.h"

namespace Fwk {

template <typename T>
class ConcurrentDeque : public Fwk::PtrInterface<ConcurrentDeque<T> > {
public:
  typedef Fwk::Ptr<ConcurrentDeque<T> const> PtrConst;
  typedef Fwk::Ptr<ConcurrentDeque<T> > Ptr;

  static Ptr concurrentDequeNew() { return new ConcurrentDeque(); }
  ConcurrentDeque() { }

  virtual ~ConcurrentDeque() { }

  /* accessors */
  size_t size() const { return deque_.size(); }
  bool empty() const { return deque_.empty(); }

  T element(uint32_t _i) {
    boost::lock_guard<boost::mutex> lock(mutex_);
    return deque_[_i];
  }

  T front() {
    boost::lock_guard<boost::mutex> lock(mutex_);
    return deque_.front();
  }

  T back() {
    boost::lock_guard<boost::mutex> lock(mutex_);
    return deque_.back();
  }

  const T element(uint32_t _i) const { return element(_i); }
  const T front() const { return front(); }
  const T back() const { return back(); }

  /* mutators */
  void elementIs(uint32_t _i, const T& _e) {
    boost::lock_guard<boost::mutex> lock(mutex_);
    deque_[_i] = _e;
  }

  void pushFront(const T& _e) {
    boost::lock_guard<boost::mutex> lock(mutex_);
    deque_.pushFront(_e);
    cond_.notify_one();
  }

  void pushBack(const T& _e) {
    boost::lock_guard<boost::mutex> lock(mutex_);
    deque_.pushBack(_e);
    cond_.notify_one();
  }

  T popFront() {
    boost::unique_lock<boost::mutex> lock(mutex_);
    while (deque_.empty())
      cond_.wait(lock);
    T e = deque_.front();
    deque_.popFront();
    return e;
  }

  T popBack() {
    boost::unique_lock<boost::mutex> lock(mutex_);
    while (deque_.empty())
      cond_.wait(lock);
    T e = deque_.back();
    deque_.popBack();
    return e;
  }

  void clear() {
    boost::lock_guard<boost::mutex> lock(mutex_);
    deque_.clear();
  }

protected:
  Deque<T> deque_;
  boost::condition_variable cond_;
  boost::mutex mutex_;
};

}

#endif
