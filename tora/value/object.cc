#include "object.h"
#include "../vm.h"
#include "code.h"
#include "class.h"
#include "../frame.h"
#include "../symbols.gen.h"

using namespace tora;

ObjectValue::ObjectValue(VM *v, const SharedPtr<ClassValue>& klass, const SharedPtr<Value>& data) : Value(VALUE_TYPE_OBJECT) {
    assert(klass.get());
    object_value_ = new ObjectImpl(v, klass, data);
}

ObjectValue::~ObjectValue() {
    delete object_value_;
}

const char *ObjectValue::type_str() const {
    return this->class_value()->name().c_str();
}

void ObjectValue::release() {
    --refcnt;
    if (refcnt == 0) {
        if (!VAL().destroyed_) {
            VAL().destroyed_ = true;
            this->call_destroy();
            assert(this->refcnt == 0);
            delete this;
        }
    }
}

void ObjectValue::dump(int indent) {
    print_indent(indent);
    printf("[dump] Object: %s(refcnt: %d)\n", this->type_str(), this->refcnt);
}

SharedPtr<Value> ObjectValue::set_item(SharedPtr<Value>index, SharedPtr<Value>v) {
    auto iter = VAL().klass_->find_method(SYMBOL___SET_ITEM__);
    if (iter != VAL().klass_->end()) {
        SharedPtr<Value>code_v = iter->second;
        assert(code_v->value_type == VALUE_TYPE_CODE);
        SharedPtr<CodeValue> code = code_v->upcast<CodeValue>();
        if (code->is_native()) {
            // TODO: use function frame
            // SharedPtr<FunctionFrame> fframe = new FunctionFrame(argcnt, this->VAL().vm_->frame_stack->back());
            // this->VAL().vm_->frame_stack->push_back(fframe);

            if (code->callback()->argc == 2) {
                // code->callback()->func2(this, index.get());
                abort(); // not tested yet.
            } else if (code->callback()->argc == -5) {
                SharedPtr<Value> ret = code->callback()->func_vm3(VAL().vm_, this, index.get(), v.get());
                return ret;
            } else {
                // this is just a warnings?
                throw new ExceptionValue("%s::__setitem__ method requires 2 arguments. This is not allowed.\n", this->type_str());
            }

            // this->VAL().vm_->frame_stack->pop_back();
        } else {
            printf("NOT IMPLEMENTED YET\n");
            abort();
       //   {
       //       SharedPtr<FunctionFrame> fframe = new FunctionFrame(argcnt, frame_stack->back());
       //       fframe->return_address = pc;
       //       fframe->orig_ops = ops;
       //       fframe->top = stack.size() - argcnt;
       //       fframe->code = code;

       //       pc = -1;
       //       this->ops = code->code_opcodes;

       //       // TODO: vargs support
       //       // TODO: kwargs support
       //       assert(argcnt == (int)code->code_params->size());
       //       for (int i=0; i<argcnt; i++) {
       //           SharedPtr<Value> arg = stack.pop();
       //           fframe->setVar(i, arg);
       //       }
       //       frame_stack->push_back(fframe);
       //   }
        }
    } else {
        throw new ExceptionValue("This is not a container type: %s\n", this->type_str());
    }
}

SharedPtr<Value> ObjectValue::get_item(SharedPtr<Value> index) {
    assert(VAL().klass_.get());
    auto iter = VAL().klass_->find_method(SYMBOL___GET_ITEM__);
    if (iter != VAL().klass_->end()) {
        SharedPtr<Value>code_v = iter->second;
        assert(code_v->value_type == VALUE_TYPE_CODE);
        SharedPtr<CodeValue> code = code_v->upcast<CodeValue>();
        if (code->is_native()) {
            // TODO: use function frame
            // SharedPtr<FunctionFrame> fframe = new FunctionFrame(argcnt, this->VAL().vm_->frame_stack->back());
            // this->VAL().vm_->frame_stack->push_back(fframe);

            if (code->callback()->argc == 2) {
                // code->callback()->func2(this, index.get());
                abort(); // not tested yet.
            } else if (code->callback()->argc == -4) {
                SharedPtr<Value> ret = code->callback()->func_vm2(VAL().vm_, this, index.get());
                return ret;
            } else {
                // this is just a warnings?
                throw new ExceptionValue("%s::DESTROY method requires arguments. This is not allowed.\n", this->type_str());
            }

            // this->VAL().vm_->frame_stack->pop_back();
        } else {
            TODO();
       //   {
       //       SharedPtr<FunctionFrame> fframe = new FunctionFrame(argcnt, frame_stack->back());
       //       fframe->return_address = pc;
       //       fframe->orig_ops = ops;
       //       fframe->top = stack.size() - argcnt;
       //       fframe->code = code;

       //       pc = -1;
       //       this->ops = code->code_opcodes;

       //       // TODO: vargs support
       //       // TODO: kwargs support
       //       assert(argcnt == (int)code->code_params->size());
       //       for (int i=0; i<argcnt; i++) {
       //           SharedPtr<Value> arg = stack.pop();
       //           fframe->setVar(i, arg);
       //       }
       //       frame_stack->push_back(fframe);
       //   }
        }
    } else {
        throw new ExceptionValue("This is not a container type: %s\n", this->type_str());
    }
}

// call DESTROY method if it's available.
void ObjectValue::call_destroy() {
    auto iter = VAL().klass_->find_method(SYMBOL_DESTROY);
    if (iter != VAL().klass_->end()) {
        SharedPtr<Value>code_v = iter->second;
        assert(code_v->value_type == VALUE_TYPE_CODE);
        SharedPtr<CodeValue> code = code_v->upcast<CodeValue>();
        if (code->is_native()) {
            if (code->callback()->argc == -3) {
                // TODO: catch exception
                SharedPtr<Value> ret = code->callback()->func_vm1(VAL().vm_, this);
            } else {
                // this is just a warnings?
                throw new ExceptionValue("%s::DESTROY method requires arguments. This is not allowed.\n", this->type_str());
            }
            // this->VAL().vm_->frame_stack->pop_back();
        } else {
            this->VAL().vm_->function_call_ex(0, code, this);
            VAL().vm_->stack.pop_back(); // trash stack top.
        }
    }

    // TODO: call AUTOLOAD?
}

bool ObjectValue::isa(ID target_id) const {
    return this->class_value()->isa(target_id);
}

