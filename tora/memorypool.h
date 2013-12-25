/// from Efficient c++ section 6

#ifndef MEMORY_POOL_H_
#define MEMORY_POOL_H_

#include <memory>

const int DEFAULT_EXPAND_SIZE = 32;

template <class T_, size_t size_ = DEFAULT_EXPAND_SIZE>
class MemoryPool {
 public:
  MemoryPool() { expandTheFreeList(size_); }
  ~MemoryPool() {
    MemoryPool<T_, size_>* nextPtr = next_;
    for (nextPtr = next_; nextPtr != 0; nextPtr = next_) {
      next_ = next_->next_;
      delete[] nextPtr;
    }
  }

  void* alloc(size_t) {
    if (!next_) expandTheFreeList(size_);

    MemoryPool<T_, size_>* head = next_;
    next_ = head->next_;

    return head;
  }
  void free(void* doomed) {
    MemoryPool<T_, size_>* head = static_cast<MemoryPool<T_, size_>*>(doomed);
    head->next_ = next_;
    next_ = head;
  }

 private:
  void expandTheFreeList(int howMany);

 private:
  MemoryPool<T_, size_>* next_;
};
template <class T_, size_t size_>
void MemoryPool<T_, size_>::expandTheFreeList(int howMany) {
  size_t size = (sizeof(T_) > sizeof(MemoryPool<T_, size_>*))
                    ? sizeof(T_)
                    : sizeof(MemoryPool<T_, size_>*);

  MemoryPool<T_, size_>* runner =
      reinterpret_cast<MemoryPool<T_, size_>*>(new char[size]);

  next_ = runner;
  for (int i = 0; i < howMany; i++) {
    runner->next_ = reinterpret_cast<MemoryPool<T_, size_>*>(new char[size]);
    runner = runner->next_;
  }
  runner->next_ = 0;
}

template <class T_, size_t size_ = DEFAULT_EXPAND_SIZE>
class UseMemoryPool {
 public:
  void* operator new(size_t size) { return pool_->alloc(size); }
  void operator delete(void* doomed, size_t) { pool_->free(doomed); }

 public:
  static void initMemPool() { pool_.reset(new MemoryPool<T_, size_>); }

 private:
  static std::auto_ptr<MemoryPool<T_, size_> > pool_;
};

template <class T_, size_t size_>
std::auto_ptr<MemoryPool<T_, size_> > UseMemoryPool<T_, size_>::pool_;

#endif  // ! MEMORY_POOL_H_
