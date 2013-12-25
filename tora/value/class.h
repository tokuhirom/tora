#ifndef TORA_VALUE_CLASS_H_
#define TORA_VALUE_CLASS_H_

#include "../value.h"
#include "../callback.h"
#include <map>

namespace tora {

class ClassValue;

class ClassImpl {
    friend class ClassValue;
protected:
    VM * vm_;
    ID name_id_;
    SharedPtr<ClassValue> superclass_;
    std::map<ID, SharedPtr<Value>> methods_;
    ClassImpl(VM *vm, ID name_id)
        : vm_(vm)
        , name_id_(name_id)
        { }
};

class ClassValue: public Value {
    friend class Value;
protected:
    inline ClassImpl& VAL() {
        return *(this->class_value_);
    }
    inline const ClassImpl& VAL() const {
        return *(this->class_value_);
    }
public:
    typedef std::map<ID, SharedPtr<Value>>::const_iterator const_iterator;

    ClassValue(VM *vm, ID name_id) : Value(VALUE_TYPE_CLASS) {
        this->class_value_ = new ClassImpl(vm, name_id);
    }
    ClassValue(VM *vm, const char* name);
    ~ClassValue() {
        delete this->class_value_;
    }
    VM * vm() const { return VAL().vm_; }
    std::string name() const;
    ID name_id() const {
        return VAL().name_id_;
    }
    /**
     * register method by code value.
     */
    void add_method(const SharedPtr<Value> &code);
    /**
     * register C++ method
     */
    void add_method(const char*, const CallbackFunction*);
    /**
     * add integer constant value
     */
    void add_constant(const char*, int val);
    /**
     * register C++ method
     */
    void add_method(ID, const CallbackFunction*);
    bool has_method(ID) const;
    SharedValue get_method_list() const;
    bool isa(ID target_id) const;
    const_iterator find_method(ID id) const {
        return VAL().methods_.find(id);
    }
    const_iterator end() const {
        return VAL().methods_.end();
    }
    const SharedPtr<ClassValue> & superclass() const {
        return VAL().superclass_;
    }
    void superclass(const SharedPtr<ClassValue> &super) {
        VAL().superclass_ = super;
    }
};

}

#endif // TORA_VALUE_CLASS_H_
