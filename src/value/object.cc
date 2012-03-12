#include "object.h"
#include "../vm.h"
#include "../package.h"
#include "code.h"
#include "../frame.h"

using namespace tora;

void ObjectValue::release() {
    --refcnt;
    if (refcnt == 0) {
        if (!destroyed) {
            destroyed = true;
            this->call_destroy();
            assert(this->refcnt == 0);
            delete this;
        }
    }
}

const char *ObjectValue::type_str() const {
    return vm_->symbol_table->id2name(package_id_).c_str();
}

void ObjectValue::dump(int indent) {
    print_indent(indent);
    printf("[dump] Object: %s(refcnt: %d)\n", vm_->symbol_table->id2name(package_id_).c_str(), this->refcnt);
}

SharedPtr<Value> ObjectValue::set_item(SharedPtr<Value>index, SharedPtr<Value>v) {
    SharedPtr<Package> pkg = this->vm_->find_package(package_id_);
    auto iter = pkg->find(this->vm_->symbol_table->get_id("__setitem__"));
    if (iter != pkg->end()) {
        SharedPtr<Value>code_v = iter->second;
        assert(code_v->value_type == VALUE_TYPE_CODE);
        SharedPtr<CodeValue> code = code_v->upcast<CodeValue>();
        if (code->is_native()) {
            // TODO: use function frame
            // SharedPtr<FunctionFrame> fframe = new FunctionFrame(argcnt, this->vm_->frame_stack->back());
            // this->vm_->frame_stack->push_back(fframe);

            if (code->callback()->argc == 2) {
                // code->callback()->func2(this, index.get());
                abort(); // not tested yet.
            } else if (code->callback()->argc == -5) {
                SharedPtr<Value> ret = code->callback()->func_vm3(vm_, this, index.get(), v.get());
                return ret;
            } else {
                // this is just a warnings?
                fprintf(stderr, "%s::__setitem__ method requires 2 arguments. This is not allowed.\n", this->vm_->symbol_table->id2name(package_id_).c_str());
                return new ExceptionValue("HMM");
            }

            // this->vm_->frame_stack->pop_back();
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
        // TODO: throw exception
        printf("This is not a container type: %s\n", this->type_str());
        abort();
    }
}

SharedPtr<Value> ObjectValue::get_item(SharedPtr<Value> index) {
    SharedPtr<Package> pkg = this->vm_->find_package(package_id_);
    auto iter = pkg->find(this->vm_->symbol_table->get_id("__getitem__"));
    if (iter != pkg->end()) {
        SharedPtr<Value>code_v = iter->second;
        assert(code_v->value_type == VALUE_TYPE_CODE);
        SharedPtr<CodeValue> code = code_v->upcast<CodeValue>();
        if (code->is_native()) {
            // TODO: use function frame
            // SharedPtr<FunctionFrame> fframe = new FunctionFrame(argcnt, this->vm_->frame_stack->back());
            // this->vm_->frame_stack->push_back(fframe);

            if (code->callback()->argc == 2) {
                // code->callback()->func2(this, index.get());
                abort(); // not tested yet.
            } else if (code->callback()->argc == -4) {
                SharedPtr<Value> ret = code->callback()->func_vm2(vm_, this, index.get());
                return ret;
            } else {
                // this is just a warnings?
                fprintf(stderr, "%s::DESTROY method requires arguments. This is not allowed.\n", this->vm_->symbol_table->id2name(package_id_).c_str());
                return new ExceptionValue("HMM");
            }

            // this->vm_->frame_stack->pop_back();
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
        // TODO: throw exception
        printf("This is not a container type: %s\n", this->type_str());
        abort();
    }
}

ObjectValue::~ObjectValue() {
}

// call DESTROY method if it's available.
void ObjectValue::call_destroy() {
    SharedPtr<Package> pkg = this->vm_->find_package(package_id_);
    auto iter = pkg->find(this->vm_->symbol_table->get_id("DESTROY"));
    if (iter != pkg->end()) {
        SharedPtr<Value>code_v = iter->second;
        assert(code_v->value_type == VALUE_TYPE_CODE);
        SharedPtr<CodeValue> code = code_v->upcast<CodeValue>();
        if (code->is_native()) {
            if (code->callback()->argc == -3) {
                // TODO: catch exception
                SharedPtr<Value> ret = code->callback()->func_vm1(vm_, this);
                if (ret->value_type == VALUE_TYPE_EXCEPTION) {
                    // TODO: warn
                    abort();
                }
            } else {
                // this is just a warnings?
                fprintf(stderr, "%s::DESTROY method requires arguments. This is not allowed.\n", this->vm_->symbol_table->id2name(package_id_).c_str());
                return;
            }
            // this->vm_->frame_stack->pop_back();
        } else {
            int argcnt = 0;
            size_t pc = this->vm_->pc;
            // TODO: catch exceptions in destroy
            SharedPtr<OPArray> end_ops = new OPArray();
            end_ops->push_back(new OP(OP_END), -1);
            this->vm_->function_call(argcnt, code, this);
            this->vm_->frame_stack->back()->upcast<FunctionFrame>()->return_address = -1;
            SharedPtr<OPArray> orig_ops = this->vm_->frame_stack->back()->upcast<FunctionFrame>()->orig_ops;
            this->vm_->frame_stack->back()->upcast<FunctionFrame>()->orig_ops = end_ops;
            this->vm_->pc = 0;
            this->vm_->execute();

            // restore
            // TODO: restore variables by RAII
            this->vm_->pc = pc;
            this->vm_->ops = orig_ops;
        }
    }

    // TODO: call AUTOLOAD?
}

