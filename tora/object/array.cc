#include "../object.h"
#include "../vm.h"
#include "../value/array.h"
#include "../value/code.h"
#include "../value/class.h"
#include "../peek.h"
#include "../frame.h"
#include "../symbols.gen.h"

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
static SharedPtr<Value> av_size(VM *vm, Value* self) {
    SharedPtr<IntValue> size = new IntValue(self->upcast<ArrayValue>()->size());
    return size;
}

/**
 * Array#sort()
 * 
 * Get a sorted array. This method is unstable sort(Perl5's sort function is stable sort).
 */
static SharedPtr<Value> av_sort(VM* vm, Value* self) {
    assert(self->value_type == VALUE_TYPE_ARRAY);
    // copy and sort.
    SharedPtr<ArrayValue> av = new ArrayValue(*(self->upcast<ArrayValue>()));
    av->sort();
    return av;
}

/**
 * Array#push($elem)
 * 
 * Push a object to the array. $elem put at end of array.
 *
 * Perl5: push(@array, $elem);
 */
static SharedPtr<Value> av_push(VM * vm, Value* self, Value* v) {
    // Inspector ins(vm);
    // printf("# ARRY PUSH: %p\n", v);
    self->upcast<ArrayValue>()->set_item(self->upcast<ArrayValue>()->size()-1+1, v);
    // self->upcast<ArrayValue>()->push_back(v);
    return self;
}

/**
 * $array.pop() : Any
 *
 * Pop a object from array.
 */
static SharedPtr<Value> av_pop(VM * vm, Value* self) {
    SharedPtr<Value> v = self->upcast<ArrayValue>()->back();
    self->upcast<ArrayValue>()->pop_back();
    return v;
}

/**
 * $array.unshift($v)
 *
 * push $v to front.
 */
static SharedPtr<Value> av_unshift(VM * vm, Value* self, Value *v) {
    self->upcast<ArrayValue>()->push_front(
        v
    );
    return UndefValue::instance();
}

/**
 * $array.shift() : Any
 *
 * pop value from front.
 */
static SharedPtr<Value> av_shift(VM * vm, Value* self) {
    if (self->upcast<ArrayValue>()->size() > 0) {
        SharedPtr<Value> ret = self->upcast<ArrayValue>()->at(0);
        self->upcast<ArrayValue>()->pop_front();
        return ret;
    } else {
        return UndefValue::instance();
    }
}

static SharedPtr<Value> av_reverse(VM * vm, Value* self) {
    ArrayValue * src = self->upcast<ArrayValue>();
    SharedPtr<ArrayValue> nav = new ArrayValue(*src);
    std::reverse(nav->begin(), nav->end());
    return nav.get();
}

/**
 * ['hoge', 'fuga'].join(' ')
 *
 * concatenete strings in array.
 */
static SharedPtr<Value> av_join(VM * vm, Value* self, Value *inner) {
    assert(self->value_type == VALUE_TYPE_ARRAY);
    SharedPtr<ArrayValue> src = self->upcast<ArrayValue>();
    SharedPtr<StrValue> str = inner->to_s();
    std::string ret;
    for (auto iter=src->begin(); iter!=src->end(); ++iter) {
        ret += (*iter)->to_s()->str_value();
        if (iter+1 != src->end()) {
            ret += str.get()->str_value();
        }
    }
    return new StrValue(ret);
}

/**
 * [1,2,3].map(-> $n { say($n) })
 */
static SharedPtr<Value> av_map(VM * vm, Value* self, Value *code_v) {
    assert(self->value_type == VALUE_TYPE_ARRAY);
    SharedPtr<ArrayValue> src = self->upcast<ArrayValue>();
    SharedPtr<ArrayValue> ret = new ArrayValue();
    if (code_v->value_type != VALUE_TYPE_CODE) {
        throw new ExceptionValue("Code is not a foo");
    }
    SharedPtr<CodeValue> code = code_v->upcast<CodeValue>();
    for (auto iter=src->begin(); iter!=src->end(); ++iter) {
        vm->stack.push_back(*iter);

        vm->function_call_ex(1, code, UndefValue::instance());

        ret->push_back(vm->stack.back());
        vm->stack.pop_back();
    }
    return ret;
}

/**
 * $array.capacity() : Int
 *
 * returns the number of elements that can be held in currently allocated storage
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
    return UndefValue::instance();
}
*/

void tora::Init_Array(VM* vm) {
    SharedPtr<ClassValue> klass = new ClassValue(vm, SYMBOL_ARRAY_CLASS);
    klass->add_method(vm->symbol_table->get_id("size"), new CallbackFunction(av_size));
    klass->add_method(vm->symbol_table->get_id("sort"), new CallbackFunction(av_sort));
    klass->add_method(vm->symbol_table->get_id("push"), new CallbackFunction(av_push));
    klass->add_method(vm->symbol_table->get_id("pop"), new CallbackFunction(av_pop));
    klass->add_method(vm->symbol_table->get_id("unshift"), new CallbackFunction(av_unshift));
    klass->add_method(vm->symbol_table->get_id("shift"), new CallbackFunction(av_shift));
    klass->add_method(vm->symbol_table->get_id("reverse"), new CallbackFunction(av_reverse));
    klass->add_method(vm->symbol_table->get_id("join"), new CallbackFunction(av_join));
    klass->add_method(vm->symbol_table->get_id("map"), new CallbackFunction(av_map));
    // klass->add_method(vm->symbol_table->get_id("capacity"), new CallbackFunction(av_capacity));
    // klass->add_method(vm->symbol_table->get_id("reserve"), new CallbackFunction(av_reserve));
    vm->add_builtin_class(klass);
}

