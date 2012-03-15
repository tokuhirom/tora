#ifndef TORA_PACKAGE_H_
#define TORA_PACKAGE_H_

#include "tora.h"
#include "value.h"
#include "shared_ptr.h"
#include "prim.h"
#include <map>

namespace tora {

struct CallbackFunction;
class VM;

class Package {
    PRIM_DECL(Package);
private:
    ID name_id;
    std::map<ID, SharedPtr<Value>> data;
public:
    typedef std::map<ID, SharedPtr<Value>>::iterator iterator;

    Package(ID id) : refcnt(0), name_id(id) { }
    ~Package () { }
    void add_function(ID function_name_id, const SharedPtr<Value> &code);

    void add_method(ID function_name_id, const CallbackFunction* code);
    void has_method(ID function_name_id, const CallbackFunction* code);
    void set_variable(ID var_name_id, const SharedPtr<Value> & v);

    iterator find(ID id) {
        return data.find(id);
    }
    void dump(VM *vm, int indent);
    void dump(VM *vm) {
        this->dump(vm, 0);
    }
    const char *type_str() { return "package"; }
    ID id() { return name_id; }
    iterator begin() { return data.begin(); }
    iterator end()   { return data.end(); }
};

};

#endif // TORA_PACKAGE_H_
