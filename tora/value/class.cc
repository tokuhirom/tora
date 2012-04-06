#include "class.h"
#include "code.h"
#include "../vm.h"
#include <utility>

using namespace tora;

ClassValue::ClassValue(VM *vm, const char* name) : Value(VALUE_TYPE_CLASS) {
    this->class_value_ = new ClassImpl(vm, vm->get_id(name));
}

std::string ClassValue::name() const {
    return VAL().vm_->symbol_table->id2name(
        VAL().name_id_
    );
}

void ClassValue::add_method(const SharedPtr<Value>& code) {
    VAL().methods_.insert(
        std::map<ID, SharedPtr<Value>>::value_type(code->upcast<CodeValue>()->func_name_id(), code)
    );
}

void ClassValue::add_method(ID name_id, const CallbackFunction*cb) {
    VAL().methods_.insert(
        std::map<ID, SharedPtr<Value>>::value_type(
            name_id,
            new CodeValue(this->name_id(), name_id, cb)
        )
    );
}

void ClassValue::add_method(const char* name, const CallbackFunction*cb) {
    VM * vm = VAL().vm_;
    ID name_id = vm->symbol_table->get_id(name);
    this->add_method(name_id, cb);
}

void ClassValue::add_constant(const char* name, int val) {
    this->add_method(name, new CallbackFunction(val));
}

bool ClassValue::isa(ID target_id) const {
    if (this->name_id() == target_id) {
        return true;
    } else {
        const SharedPtr<ClassValue> &parent = VAL().superclass_;
        if (parent.get()) {
            return parent->isa(target_id);
        } else {
            return false;
        }
    }
}

bool ClassValue::has_method(ID target_id) const {
    return VAL().methods_.find(target_id) != VAL().methods_.end();
}

SharedPtr<ArrayValue> ClassValue::get_method_list() const {
    SharedPtr<ArrayValue> av = new ArrayValue();
    for (auto iter=VAL().methods_.begin(); iter!=VAL().methods_.end(); ++iter) {
        av->push_back(new StrValue(VAL().vm_->symbol_table->id2name(iter->first)));
    }
    return av;
}

