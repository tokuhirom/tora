#include "nanotap.h"
#include "../tora/frame.h"
#include "../tora/value/int.h"
#include <stdarg.h>
#include <assert.h>

using namespace tora;

int main() {
    std::string filename_("foo.tra");

    {
        SharedPtr<PadList> p1 = new PadList(1, NULL);
        MortalIntValue i5(5);
        p1->set(0, i5.get());
        SharedPtr<Value> val = p1->get(0);
        ok(!!val.get());
        is(val->value_type, VALUE_TYPE_INT);
        is(val->to_int(), 5);

        SharedPtr<Value> val1 = p1->get(1);
        is(val1->value_type, VALUE_TYPE_UNDEF);
    }

    printf("# second phase\n");
    {
        SharedPtr<PadList> p1 = new PadList(0, NULL);
        SharedPtr<PadList> p2 = new PadList(0, p1.get());
        SharedPtr<PadList> p3 = new PadList(0, p2.get());
        is((void*)p1->upper(1), (void*)NULL);
        is(p2->upper(1), p1.get());
        is(p3->upper(1), p2.get());
        is(p3->upper(2), p1.get());
        is((void*)p3->upper(3), (void*)NULL);
    }

    printf("# third phase\n");

    {
        SharedPtr<PadList> p1 = new PadList(1, NULL);
        SharedPtr<PadList> p2 = new PadList(1, p1.get());
        SharedPtr<PadList> p3 = new PadList(3, p2.get());
        MortalIntValue i5(5);
        MortalIntValue i9(9);
        MortalIntValue i6(6);
        MortalIntValue i3(3);
        p1->set(0, i5.get());
        p2->set(1, i9.get());
        p3->set(2, i6.get());
        p3->set(0, i3.get());

        is(p2->get_dynamic(1, 0)->value_type, VALUE_TYPE_INT);
        is(p2->get_dynamic(1, 0)->to_int(), 5);
        is(p3->get_dynamic(2, 0)->to_int(), 5);
        is(p3->get_dynamic(1, 1)->to_int(), 9);
    }

    done_testing();
}
