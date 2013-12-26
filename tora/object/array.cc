#include "../object.h"
#include "../vm.h"
#include "../value/array.h"
#include "../value/code.h"
#include "../value/class.h"
#include "../value/regexp.h"
#include "../peek.h"
#include "../frame.h"
#include "../symbols.gen.h"
#include "../class_builder.h"

using namespace tora;

/**
 * class Array
 *
 * Array class is an array class.
 *
 * You can construct an Array instance from array literal like:
 * <pre>[1,2,3, "hoge"]</pre>
 * Array can contains any objects.
 */

/**
 * Array#size()
 *
 * Get the number of elements in an array.
 */
static SharedPtr<Value> av_size(VM* vm, Value* self) {
  SharedPtr<IntValue> size = new IntValue(array_size(self));
  return size;
}

/**
 * Array#sort()
 *
 * Get a sorted array. This method is stable sort(Because Perl5's sort function
 *is stable).
 */
static SharedPtr<Value> av_sort(VM* vm, Value* self) {
  assert(self->value_type == VALUE_TYPE_ARRAY);
  // copy and sort.
  SharedValue av(array_stable_sort(self));
  return av.get();
}

/**
 * Array#push($elem)
 *
 * Push a object to the array. $elem put at end of array.
 *
 * Perl5: push(@array, $elem);
 */
static SharedPtr<Value> av_push(VM* vm, Value* self, Value* v) {
  // Inspector ins(vm);
  // printf("# ARRY PUSH: %p\n", v);
  array_set_item(self, array_size(self) - 1 + 1, v);
  // self->upcast<ArrayValue>()->push_back(v);
  return self;
}

/**
 * $array.pop() : Any
 *
 * Pop a object from array.
 */
static SharedPtr<Value> av_pop(VM* vm, Value* self) {
  SharedValue v = array_back(self);
  array_pop_back(self);
  return v.get();
}

/**
 * $array.unshift($v)
 *
 * push $v to front.
 */
static SharedPtr<Value> av_unshift(VM* vm, Value* self, Value* v) {
  array_push_front(self, v);
  return new_undef_value();
}

/**
 * $array.shift() : Any
 *
 * pop value from front.
 */
static SharedPtr<Value> av_shift(VM* vm, Value* self) {
  if (array_size(self) > 0) {
    SharedValue ret = array_get_item(self, 0);
    array_pop_front(self);
    return ret.get();
  } else {
    return new_undef_value();
  }
}

static SharedPtr<Value> av_reverse(VM* vm, Value* self) {
  return array_reverse(self).get();
}

/**
 * ['hoge', 'fuga'].join(' ')
 *
 * concatenete strings in array.
 */
static SharedPtr<Value> av_join(VM* vm, Value* self, Value* inner) {
  assert(self->value_type == VALUE_TYPE_ARRAY);
  std::string str = inner->to_s();
  std::string ret;
  for (tra_int i = 0, l = array_size(self); i < l; ++i) {
    ret += array_get_item(self, i)->to_s();
    if (i != l - 1) {
      ret += str;
    }
  }
  return new_str_value(ret);
}

/**
 * [1,2,3].map(-> $n { say($n) })
 */
static SharedPtr<Value> av_map(VM* vm, Value* self, Value* code_v) {
  assert(self->value_type == VALUE_TYPE_ARRAY);
  MortalArrayValue ret;
  if (code_v->value_type != VALUE_TYPE_CODE) {
    throw new ExceptionValue("Code is not a foo");
  }
  SharedPtr<CodeValue> code = code_v->upcast<CodeValue>();
  for (tra_int i = 0, l = array_size(self); i < l; ++i) {
    vm->stack.push_back(array_get_item(self, i));

    vm->function_call_ex(1, code, new_undef_value());

    array_push_back(ret.get(), vm->stack.back().get());
    vm->stack.pop_back();
  }
  return ret.get();
}

/**
 * $array.grep(Code $code) : Array
 *
 * Filter $array by $code.
 */
static SharedPtr<Value> av_grep(VM* vm, Value* self, Value* stuff_v) {
  assert(self->value_type == VALUE_TYPE_ARRAY);
  MortalArrayValue ret;
  if (stuff_v->value_type == VALUE_TYPE_CODE) {
    SharedPtr<CodeValue> code = stuff_v->upcast<CodeValue>();
    for (tra_int i = 0, l = array_size(self); i < l; ++i) {
      vm->stack.push_back(array_get_item(self, i));

      vm->function_call_ex(1, code, new_undef_value());

      if (vm->stack.back()->to_bool()) {
        array_push_back(ret.get(), array_get_item(self, i));
      }
      vm->stack.pop_back();
    }
    return ret.get();
  } else if (stuff_v->value_type == VALUE_TYPE_REGEXP) {
    SharedPtr<AbstractRegexpValue> re = stuff_v->upcast<AbstractRegexpValue>();
    for (tra_int i = 0, l = array_size(self); i < l; ++i) {
      std::string str = array_get_item(self, i)->to_s();

      if (re->match_bool(vm, str)) {
        array_push_back(ret.get(), array_get_item(self, i));
      }
    }
    return ret.get();
  } else {
    throw new ExceptionValue(
        "Second argument for Array#grep should be Code or Regexp");
  }
}

/**
 * $array.capacity() : Int
 *
 * returns the number of elements that can be held in currently allocated
 *storage
 */
/*
static SharedPtr<Value> av_capacity(VM * vm, Value* self) {
    return new IntValue(self->upcast<ArrayValue>()->values->capacity());
}
*/

/**
 * $array.reserve($n)
 *
 * reserves storage
 */
/*
static SharedPtr<Value> av_reserve(VM * vm, Value* self, Value * v) {
    self->upcast<ArrayValue>()->values->reserve(v->to_int());
    return new_undef_value();
}
*/

void tora::Init_Array(VM* vm) {
  ClassBuilder builder(vm, SYMBOL_ARRAY_CLASS);
  builder.add_method("size", new CallbackFunction(av_size));
  builder.add_method("sort", new CallbackFunction(av_sort));
  builder.add_method("push", new CallbackFunction(av_push));
  builder.add_method("pop", new CallbackFunction(av_pop));
  builder.add_method("unshift", new CallbackFunction(av_unshift));
  builder.add_method("shift", new CallbackFunction(av_shift));
  builder.add_method("reverse", new CallbackFunction(av_reverse));
  builder.add_method("join", new CallbackFunction(av_join));
  builder.add_method("map", new CallbackFunction(av_map));
  builder.add_method("grep", new CallbackFunction(av_grep));
  // builder.add_method("capacity"), new
  // CallbackFunction(av_capacity));
  // builder.add_method("reserve"), new
  // CallbackFunction(av_reserve));
  vm->add_builtin_class(builder.value());
}
