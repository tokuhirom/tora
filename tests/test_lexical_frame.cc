#include "nanotap.h"
#include "../tora/frame.h"
#include "../tora/vm.h"
#include "../tora/symbol_table.h"
#include "../tora/op_array.h"
#include "../tora/value/int.h"
#include <stdarg.h>
#include <assert.h>

using namespace tora;

int main() {
    {
        std::shared_ptr<OPArray> opa(new OPArray());
        SharedPtr<SymbolTable> st(new SymbolTable());
        VM vm(opa, st, false);
        int vars_cnt = 3;
        int top = 0;
    // LexicalVarsFrame(VM *vm, int vars_cnt, size_t top, frame_type_t type_=FRAME_TYPE_LEXICAL);
        SharedPtr<LexicalVarsFrame> f1(new LexicalVarsFrame(&vm, vars_cnt, top));
        vm.frame_stack->push_back(f1);
        SharedPtr<LexicalVarsFrame> f2(new LexicalVarsFrame(&vm, vars_cnt, top));
        vm.frame_stack->push_back(f2);
        SharedPtr<Value> v = f2->get_variable_dynamic(1, 0);
        is(v->value_type, VALUE_TYPE_UNDEF);
    }

    {
        std::shared_ptr<OPArray> opa(new OPArray());
        SharedPtr<SymbolTable> st(new SymbolTable());
        VM vm(opa, st, false);
        int vars_cnt = 3;
        int top = 0;
    // LexicalVarsFrame(VM *vm, int vars_cnt, size_t top, frame_type_t type_=FRAME_TYPE_LEXICAL);
        SharedPtr<LexicalVarsFrame> f1(new LexicalVarsFrame(&vm, vars_cnt, top));
        vm.frame_stack->push_back(f1);
        MortalIntValue i(5);
        f1->set_variable(0, i.get());
        is(f1->get_variable(0)->to_int(), 5);
        MortalIntValue i9(9);
        f1->set_variable(0, i9.get());
        is(f1->get_variable(0)->to_int(), 9);
    }

    {
        std::shared_ptr<OPArray> opa(new OPArray());
        SharedPtr<SymbolTable> st(new SymbolTable());
        VM vm(opa, st, false);
        int vars_cnt = 3;
        int top = 0;
    // LexicalVarsFrame(VM *vm, int vars_cnt, size_t top, frame_type_t type_=FRAME_TYPE_LEXICAL);
        SharedPtr<LexicalVarsFrame> f1(new LexicalVarsFrame(&vm, vars_cnt, top));
        vm.frame_stack->push_back(f1);
        SharedPtr<LexicalVarsFrame> f2(new LexicalVarsFrame(&vm, vars_cnt, top));
        vm.frame_stack->push_back(f2);
        MortalIntValue i4(4);
        f2->set_variable(0, i4.get());
        MortalIntValue i5(5);
        f1->set_variable(0, i5.get());
        is(f2->get_variable(0)->to_int(), 4);
        is(f2->get_variable_dynamic(1, 0)->to_int(), 5);
        SharedPtr<Value> v = new_str_value("OOO");
        f2->set_variable_dynamic(1, 0, v);
        is(f2->get_variable_dynamic(1, 0)->to_s(), v->to_s());
    }

    done_testing();
}
