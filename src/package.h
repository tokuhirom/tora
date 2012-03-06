#ifndef TORA_PACKAGE_H_
#define TORA_PACKAGE_H_

#include "value.h"

namespace tora {

struct CallbackFunction;

class Package : public Value {
    ID name_id;
    std::map<ID, SharedPtr<Value>> data;
public:
    typedef std::map<ID, SharedPtr<Value>>::iterator iterator;

    Package(ID id) : Value(VALUE_TYPE_PACKAGE), name_id(id) { }
    ~Package () { }
    void add_function(ID function_name_id, SharedPtr<Value> code);

    void add_method(ID function_name_id, const CallbackFunction* code);

    iterator find(ID id) {
        return data.find(id);
    }
    void dump(SharedPtr<SymbolTable> & symbol_table, int indent);
    virtual void dump(int indent) {
        print_indent(indent);
        printf("[dump] Package\n");
    }
    const char *type_str() { return "package"; }
    ID id() { return name_id; }
    iterator begin() { return data.begin(); }
    iterator end()   { return data.end(); }
};

};

#endif // TORA_PACKAGE_H_
