#ifndef TORA_HASH_H_
#define TORA_HASH_H_

#include "../value.h"

namespace tora {

  class MortalHashValue : public MortalValue {
    static Value* new_hash_value();
  public:
    MortalHashValue() : MortalValue(new_hash_value()) { }
  };

  class MortalHashIteratorValue : public MortalValue {
    static Value* new_value(Value *h);
  public:
    MortalHashIteratorValue(Value *h) : MortalValue(new_value(h)) { }
  };

  void hash_delete_key(Value* v, const std::string &key);
  bool hash_has_key(const Value* v, const std::string &key);
  size_t hash_size(const Value* v);
  Value* hash_get_item(Value* v, const std::string& index);
  void hash_set_item(Value* v, const std::string & key, Value* val);

  bool hash_iter_finished(Value* v);
  std::string hash_iter_getkey(Value* v);
  Value* hash_iter_getval(Value* v);
  void hash_iter_increment(Value* v);

};

#endif // TORA_HASH_H_
