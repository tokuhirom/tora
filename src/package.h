#ifndef TORA_PACKAGE_H_
#define TORA_PACKAGE_H_

#include "value.h"

namespace tora {

class CallbackFunction;

class Package {
    PRIM_DECL(Package);
private:
    ID name_id;
    std::map<ID, SharedPtr<Value>> data;
public:
    typedef std::map<ID, SharedPtr<Value>>::iterator iterator;

    Package(ID id) : refcnt(0), name_id(id) { }
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
