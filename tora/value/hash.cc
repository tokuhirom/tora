#include "hash.h"
#include <map>

using namespace tora;

typedef std::map<std::string, SharedValue> HashImpl;

static HashImpl* hash(const Value* v) {
  assert(type(v) == VALUE_TYPE_HASH);
  return static_cast<HashImpl*>(get_ptr_value(v));
}

// -- iter class --

namespace tora {
struct HashIteratorImpl {
  HashImpl::iterator iter;
  SharedValue parent;
  HashIteratorImpl(Value* p) {
    parent = p;
    iter = hash(p)->begin();
  }
};
}

static HashIteratorImpl* hash_iter(const Value* v) {
  assert(type(v) == VALUE_TYPE_HASH_ITERATOR);
  return static_cast<HashIteratorImpl*>(get_ptr_value(v));
}

// -- MortalHashValue --

Value* MortalHashValue::new_hash_value() {
  Value* v = new Value(VALUE_TYPE_HASH);
  v->ptr_value_ = new HashImpl();
  return v;
}

// -- Hash --

void tora::hash_delete_key(Value* v, const std::string& key) {
  auto iter = hash(v)->find(key);
  if (iter != hash(v)->end()) {
    hash(v)->erase(iter);
  }
}

bool tora::hash_has_key(const Value* v, const std::string& key) {
  return hash(v)->find(key) != hash(v)->end();
}

size_t tora::hash_size(const Value* v) { return hash(v)->size(); }

Value* tora::hash_get_item(Value* v, const std::string& index) {
  return hash(v)->at(index).get();
}

void tora::hash_set_item(Value* v, const std::string& key, Value* val) {
  (*hash(v))[key] = val;
}

// -- Iterator --

Value* MortalHashIteratorValue::new_value(Value* h) {
  Value* v = new Value(VALUE_TYPE_HASH_ITERATOR);
  v->ptr_value_ = new HashIteratorImpl(h);
  return v;
}

bool tora::hash_iter_finished(Value* v) {
  return hash_iter(v)->iter == hash(hash_iter(v)->parent.get())->end();
}

std::string tora::hash_iter_getkey(Value* v) {
  return hash_iter(v)->iter->first;
}

Value* tora::hash_iter_getval(Value* v) {
  return hash_iter(v)->iter->second.get();
}

void tora::hash_iter_increment(Value* v) { hash_iter(v)->iter++; }
