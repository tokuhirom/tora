#include "../src/stack.h"
#include "../src/value.h"
#include "nanotap.h"

using namespace tora;

int main() {
    {
        Stack s(3);
        is(s.capacity(), (size_t)3);
        is(s.size(), (size_t)0);
        for (int i=0; i<100; i++) {
            s.push(UndefValue::instance());
        }
        is(s.capacity(), (size_t)192);
        is(s.size(), (size_t)100);
        for (int i=0; i<100; i++) {
            is(s.top()->value_type, VALUE_TYPE_UNDEF);
            s.pop_back();
        }
        is(s.capacity(), (size_t)192); // do not decrease stack size automatically.
        is(s.size(), (size_t)0);
    }

    {
        Stack s(3);
        for (int i=0; i<100; i++) {
            s.push(UndefValue::instance());
        }
        is(s.capacity(), (size_t)192);
        is(s.size(), (size_t)100);
        s.resize(10);
        is(s.capacity(), (size_t)192);
        is(s.size(), (size_t)10);
    }

    done_testing();
}

