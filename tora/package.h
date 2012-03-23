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
    VM *vm_;
    ID name_id;
    Package * superclass_;
    std::map<ID, SharedPtr<Value>> data;
public:
    typedef std::map<ID, SharedPtr<Value>>::iterator iterator;

    Package(VM *vm, ID id) : refcnt(0), vm_(vm), name_id(id), superclass_(NULL) { }
    ~Package ();
    void add_function(ID function_name_id, const SharedPtr<Value> &code);

    void add_method(ID function_name_id, const CallbackFunction* code);
    void add_method(const std::string& function_name_id, const CallbackFunction* code);
    bool has_method(ID function_name_id);
    void set_variable(ID var_name_id, const SharedPtr<Value> & v);
    void add_constant(const std::string &, int n);
    void add_constant(ID function_name_id, int n);

    void superclass(Package * super) {
        superclass_ = super;
    }
    Package* superclass() const {
        return superclass_;
    }

    iterator find(ID id) {
        return data.find(id);
    }
    void dump(VM *vm, int indent);
    void dump(VM *vm) {
        this->dump(vm, 0);
    }
    const char *type_str() const { return "package"; }
    ID id() const { return name_id; }
    iterator begin() { return data.begin(); }
    iterator end()   { return data.end(); }
};

};

#endif // TORA_PACKAGE_H_
