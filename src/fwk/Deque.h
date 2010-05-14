/** \file Deque.h
 * Double-ended queue data structure.
 * \author Ali Yahya
 * \author Matt Sparks
 */
#ifndef __FWK__DEQUE_H__
#define __FWK__DEQUE_H__

#include <deque>
#include <boost/utility.hpp>

#include "Ptr.h"
#include "PtrInterface.h"

namespace Fwk {

template <typename T>
class Deque : public Fwk::PtrInterface<Deque<T> >, boost::noncopyable {
public:
  typedef Fwk::Ptr<Deque<T> const> PtrConst;
  typedef Fwk::Ptr<Deque<T> > Ptr;

  typedef typename std::deque<T>::iterator iterator;
  typedef typename std::deque<T>::reverse_iterator reverse_iterator;
  typedef typename std::deque<T>::const_iterator const_iterator;
  typedef typename std::deque<T>::const_reverse_iterator const_reverse_iterator;

  static Ptr dequeNew() { return new Deque(); }
  Deque() { }

  virtual ~Deque() { }

  /* iterators */
  iterator begin() { return deque_.begin(); }
  iterator end() { return deque_.end(); }
  reverse_iterator rbegin() { return deque_.rbegin(); }
  reverse_iterator rend() { return deque_.rend(); }

  const_iterator begin() const { return deque_.begin(); }
  const_iterator end() const { return deque_.end(); }
  const_reverse_iterator rbegin() const { return deque_.rbegin(); }
  const_reverse_iterator rend() const { return deque_.rend(); }

  /* accessors */
  size_t size() const { return deque_.size(); }
  bool empty() const { return deque_.empty(); }
  T& element(uint32_t _i) { return deque_[_i]; }
  T& front() { return deque_.front(); }
  T& back() { return deque_.back(); }

  const T& element(uint32_t _i) const { return element(_i); }
  const T& front() const { return front(); }
  const T& back() const { return back(); }

  /* mutators */
  iterator elementDel(iterator it) { return deque_.erase(it); }
  void elementIs(uint32_t _i, const T& _e) { deque_[_i] = _e; }
  void pushFront(const T& _e) { deque_.push_front(_e); }
  void pushBack(const T& _e) { deque_.push_back(_e); }
  void popFront() { deque_.pop_front(); }
  void popBack() { deque_.pop_back(); }
  void clear() { deque_.clear(); }

  /* overloaded operators */
  T& operator[](const uint32_t& _i) { return element(_i); }
  const T& operator[](const uint32_t& _i) const { return element(_i); }

protected:
  std::deque<T> deque_;
};

}

#endif
