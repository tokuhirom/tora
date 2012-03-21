#ifndef TORA_HASH_H_
#define TORA_HASH_H_

#include "../value.h"
#include "../shared_ptr.h"
#include "str.h"
#include "undef.h"

namespace tora {

class HashValue: public Value {
protected:
    std::map<std::string, SharedPtr<Value> > data;
public:
    typedef std::map<std::string, SharedPtr<Value> >::iterator iter;
    HashValue() : Value(VALUE_TYPE_HASH) { }
    SharedPtr<Value> get(const std::string &key) {
        return data[key];
    }
    void set(const std::string & key , const SharedPtr<Value>&val) {
        data[key] = val;
    }

    iter begin() { return data.begin(); }
    iter end()   { return data.end(); }

    SharedPtr<Value> set_item(SharedPtr<Value>index, SharedPtr<Value>v) {
        SharedPtr<StrValue> s = index->to_s();
        this->set(s->str_value(), v);
        return UndefValue::instance();
    }
    SharedPtr<Value> get_item(SharedPtr<Value> index) {
        SharedPtr<StrValue> s = index->to_s();
        return this->data[s->str_value()];
    }
    size_t size() {
        return data.size();
    }
    bool has_key(SharedPtr<Value> key) {
        SharedPtr<StrValue> k = key->to_s();
        return this->has_key(k->str_value());
    }
    bool has_key(const std::string & key) {
        return this->data.find(key) != this->data.end();
    }

    class iterator : public Value {
        std::map<std::string, SharedPtr<Value> >::iterator iter;
        SharedPtr<HashValue> parent;
    public:
        iterator(const SharedPtr<HashValue> & parent_) : Value(VALUE_TYPE_HASH_ITERATOR) {
            parent = parent_;
            iter = parent->data.begin();
        }
        bool finished() {
            return iter == parent->data.end();
        }
        SharedPtr<Value> getkey() {
            return new StrValue(iter->first);
        }
        SharedPtr<Value> getval() {
            return iter->second;
        }
        void increment() {
            iter++;
        }
        const char *type_str() { return "hash_iterator"; }
    };
};

};

#endif // TORA_HASH_H_
