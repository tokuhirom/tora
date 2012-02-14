#ifndef TORA_HASH_H_
#define TORA_HASH_H_

#include "../value.h"
#include "../shared_ptr.h"

namespace tora {

class HashValue: public Value {
    std::map<std::string, SharedPtr<Value> > data;
public:
    HashValue() {
    }
    SharedPtr<Value> get(const std::string &key) {
        return data[key];
    }
    void set(const std::string & key , SharedPtr<Value>&val) {
        data[key] = val;
    }
    void dump(int indent);
    const char *type_str() { return "hash"; }

    void set_item(SharedPtr<Value>index, SharedPtr<Value>v) {
        SharedPtr<StrValue> s = index->to_s();
        this->set(s->str_value, v);
    }
    SharedPtr<Value> get_item(SharedPtr<Value> index) {
        SharedPtr<StrValue> s = index->to_s();
        return this->get(s->str_value);
    }
};

};

#endif // TORA_HASH_H_
