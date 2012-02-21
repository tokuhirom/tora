#ifndef TORA_HASH_H_
#define TORA_HASH_H_

#include "../value.h"
#include "../shared_ptr.h"

namespace tora {

class HashValue: public Value {
protected:
    std::map<std::string, SharedPtr<Value> > data;
public:
    HashValue() {
        value_type = VALUE_TYPE_HASH;
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
    size_t size() {
        return data.size();
    }

    class iterator : public Value {
        std::map<std::string, SharedPtr<Value> >::iterator iter;
        SharedPtr<HashValue> parent;
    public:
        iterator(const SharedPtr<HashValue> & parent_) {
            value_type = VALUE_TYPE_HASH_ITERATOR;
            parent = parent_;
            iter = parent->data.begin();
        }
        bool finished() {
            return iter == parent->data.end();
        }
        SharedPtr<Value> get() {
            return iter->second;
        }
        void increment() {
            iter++;
        }
        void dump(int indent) {
            print_indent(indent);
            printf("[dump] hash_iterator:\n");
            parent->dump(indent+1);
        }
        const char *type_str() { return "hash_iterator"; }
    };
};

};

#endif // TORA_HASH_H_
