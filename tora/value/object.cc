#include "object.h"
#include "../vm.h"
#include "code.h"
#include "class.h"
#include "../frame.h"
#include "../symbols.gen.h"
#include "../value/exception.h"

using namespace tora;

namespace tora {
  struct ObjectImpl {
    VM* vm;
    SharedPtr<Value> klass;
    bool destroyed;
    SharedPtr<Value> data;
    ObjectImpl(VM* _vm, Value* _klass, Value* _data)
        : vm(_vm), klass(_klass), destroyed(false), data(_data) {}
  };
};

Value* MortalObjectValue::new_value(VM*vm, Value* klass, Value* data)
{
  ObjectImpl*p = new ObjectImpl(vm, klass, data);
  Value *v = new Value(VALUE_TYPE_OBJECT);
  v->ptr_value_ = p;
  return v;
}

static ObjectImpl* object(const Value* v)
{
  assert(type(v) == VALUE_TYPE_OBJECT);
  return static_cast<ObjectImpl*>(get_ptr_value(v));
}

// -- object --

void tora::object_free(Value* self)
{
  ObjectImpl *o = object(self);
  assert(o);
  delete o;

  self->ptr_value_ = NULL;
}

std::string tora::object_type_str(const Value* self)
{
  return class_name(object(self)->klass.get());
}

// call DESTROY method if it's available.
static void call_destroy(Value* self) {
  Value* code = class_get_method(object(self)->klass.get(), SYMBOL_DESTROY);
  if (code) {
    assert(code->value_type == VALUE_TYPE_CODE);
    if (code_is_native(code)) {
      if (code_callback(code)->argc == -3) {
        // TODO: catch exception
        SharedPtr<Value> ret = code_callback(code)->func_vm1(object(self)->vm, self);
      } else {
        // this is just a warnings?
        throw new ExceptionValue(
            "%s::DESTROY method requires arguments. This is not allowed.\n",
            self->type_str());
      }
      // this->VAL().vm_->frame_stack->pop_back();
    } else {
      object(self)->vm->function_call_ex(0, code, self);
      object(self)->vm->stack.pop_back();  // trash stack top.
    }
  }

  // TODO: call AUTOLOAD?
}

void tora::object_finalize(Value* self)
{
  ObjectImpl* impl = object(self);
  if (!impl->destroyed) {
    impl->destroyed = true;
    call_destroy(self);
  }
}

void tora::object_dump(Value* self, int indent)
{
  print_indent(indent);
  printf("[dump] Object: %s(refcnt: %d)\n", object_type_str(self).c_str(), self->refcnt());
}

SharedValue tora::object_set_item(Value* self, Value* index, Value* v) {
  Value* code = class_get_method(object(self)->klass.get(), SYMBOL___SET_ITEM__);
  if (code) {
    assert(code->value_type == VALUE_TYPE_CODE);
    if (code_is_native(code)) {
      // TODO: use function frame
      // SharedPtr<FunctionFrame> fframe = new FunctionFrame(argcnt,
      // this->VAL().vm_->frame_stack->back());
      // this->VAL().vm_->frame_stack->push_back(fframe);

      if (code_callback(code)->argc == 2) {
        // code->callback()->func2(this, index.get());
        abort();  // not tested yet.
      } else if (code_callback(code)->argc == -5) {
        SharedPtr<Value> ret =
            code_callback(code)->func_vm3(object(self)->vm, self, index, v);
        return ret.get();
      } else {
        // this is just a warnings?
        throw new ExceptionValue(
            "%s::__setitem__ method requires 2 arguments. This is not "
            "allowed.\n",
            self->type_str());
      }

      // this->VAL().vm_->frame_stack->pop_back();
    } else {
      printf("NOT IMPLEMENTED YET\n");
      abort();
      //   {
      //       SharedPtr<FunctionFrame> fframe = new FunctionFrame(argcnt,
      // frame_stack->back());
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
    throw new ExceptionValue("This is not a container type: %s\n",
                             self->type_str());
  }
}

SharedValue tora::object_get_item(Value* self, Value* index) {
  assert(object(self)->klass.get());
  Value* code = class_get_method(object(self)->klass.get(), SYMBOL___GET_ITEM__);
  if (code) {
    assert(code->value_type == VALUE_TYPE_CODE);
    if (code_is_native(code)) {
      // TODO: use function frame
      // SharedPtr<FunctionFrame> fframe = new FunctionFrame(argcnt,
      // this->VAL().vm_->frame_stack->back());
      // this->VAL().vm_->frame_stack->push_back(fframe);

      if (code_callback(code)->argc == 2) {
        // code->callback()->func2(this, index.get());
        abort();  // not tested yet.
      } else if (code_callback(code)->argc == -4) {
        SharedPtr<Value> ret =
            code_callback(code)->func_vm2(object(self)->vm, self, index);
        return ret.get();
      } else {
        // this is just a warnings?
        throw new ExceptionValue(
            "%s::DESTROY method requires arguments. This is not allowed.\n",
            self->type_str());
      }

      // this->VAL().vm_->frame_stack->pop_back();
    } else {
      TODO();
      //   {
      //       SharedPtr<FunctionFrame> fframe = new FunctionFrame(argcnt,
      // frame_stack->back());
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
    throw new ExceptionValue("This is not a container type: %s\n",
                             self->type_str());
  }
}


bool tora::object_isa(Value* self, ID target_id) {
  return class_isa(object(self)->klass.get(), target_id);
}

Value* tora::object_class(const Value* self)
{
  return object(self)->klass.get();
}

Value* tora::object_data(Value* self)
{
  return object(self)->data.get();
}
