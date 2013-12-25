#ifndef TORA_SHARED_PTR_H_
#define TORA_SHARED_PTR_H_

#include <assert.h>
#include <stdio.h>
#include <algorithm>
#include <utility>  // std::move

namespace tora {

/// It's my own shared pointer.
template <class T>
class SharedPtr {
  typedef SharedPtr this_type;

  T *ptr;

 public:
  SharedPtr() : ptr(NULL) {}
  SharedPtr(T *v, bool add_ref = true) : ptr(v) {
    if (v && add_ref) {
      v->retain();
    }
  }
  template <class U>
  SharedPtr(SharedPtr<U> const &rhs)
      : ptr(rhs.get()) {
    if (ptr) {
      ptr->retain();
    }
  }
  SharedPtr(SharedPtr const &rhs) : ptr(rhs.ptr) {
    if (ptr) {
      ptr->retain();
    }
  }
  ~SharedPtr() {
    if (ptr) {
      ptr->release();
    }
  }
  template <class U>
  SharedPtr &operator=(SharedPtr<U> const &rhs) {
    this_type(rhs).swap(*this);
    return *this;
  }
#ifdef move
  SharedPtr(SharedPtr &&rhs) : ptr(rhs.ptr) { rhs.ptr = 0; }
  SharedPtr &operator=(SharedPtr &&rhs) {
    this_type(std::move(rhs)).swap(*this);
    return *this;
  }
#endif

  SharedPtr &operator=(SharedPtr const &rhs) {
    this_type(rhs).swap(*this);
    return *this;
  }
  SharedPtr &operator=(T *rhs) {
    this_type(rhs).swap(*this);
    return *this;
  }

  void reset() { this_type().swap(*this); }

  void reset(T *rhs) { this_type(rhs).swap(*this); }

  T *get() const { return ptr; }
  T &operator*() const {
    assert(ptr != NULL);
    return *ptr;
  }
  T *operator->() const {
    assert(ptr != NULL);
    return ptr;
  }

  typedef T *this_type::*unspecified_bool_type;
  operator unspecified_bool_type() const  // never throws
  {
    return ptr == 0 ? 0 : &this_type::ptr;
  }

  bool operator!() const { return ptr == 0; }

  void swap(SharedPtr &rhs) {
    T *tmp = ptr;
    ptr = rhs.ptr;
    rhs.ptr = tmp;
  }
};
};

#endif  // TORA_SHARED_PTR_H_
