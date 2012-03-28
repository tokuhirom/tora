#ifndef TORA_HASH_H_
#define TORA_HASH_H_

#include "../value.h"
#include "../shared_ptr.h"
#include "str.h"
#include "undef.h"

namespace tora {

class HashValue: public Value {
private:
    inline HashImpl& VAL() {
        return *(this->hash_value_);
    }
    inline const HashImpl& VAL() const {
        return *(this->hash_value_);
    }
public:
    typedef HashImpl::iterator iter;

    HashValue() : Value(VALUE_TYPE_HASH) {
        this->hash_value_ = new HashImpl();
    }
    ~HashValue() {
        delete this->hash_value_;
    }
    void clear() {
        VAL().clear();
    }
    SharedPtr<Value> get(const std::string &key) {
        return VAL()[key];
    }
    void set(const std::string & key , const SharedPtr<Value>&val) {
        VAL()[key] = val;
    }

    iter begin() { return VAL().begin(); }
    iter end()   { return VAL().end(); }

    void set_item(SharedPtr<Value>index, SharedPtr<Value>v) {
        SharedPtr<StrValue> s = index->to_s();
        this->set(s->str_value(), v);
    }
    SharedPtr<Value> get_item(SharedPtr<Value> index) {
        SharedPtr<StrValue> s = index->to_s();
        return this->VAL()[s->str_value()];
    }
    size_t size() {
        return VAL().size();
    }
    bool has_key(const SharedPtr<Value> & key) const {
        SharedPtr<StrValue> k = key->to_s();
        return this->has_key(k->str_value());
    }
    bool has_key(const std::string & key) const {
        return this->VAL().find(key) != this->VAL().end();
    }
    void delete_key(const SharedPtr<Value> &key) {
        SharedPtr<StrValue> k = key->to_s();
        this->delete_key(k->str_value());
    }
    void delete_key(const std::string &key) {
        auto iter = this->VAL().find(key);
        if (iter != this->VAL().end()) {
            this->VAL().erase(iter);
        }
    }

    class iterator : public Value {
        std::map<std::string, SharedPtr<Value> >::iterator iter;
        SharedPtr<HashValue> parent;
    public:
        iterator(const SharedPtr<HashValue> & parent_) : Value(VALUE_TYPE_HASH_ITERATOR) {
            parent = parent_;
            iter = parent->VAL().begin();
        }
        bool finished() {
            return iter == parent->VAL().end();
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
