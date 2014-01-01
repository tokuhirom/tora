#include "nanotap.h"
#include "../tora/frame.h"
#include "../tora/value/int.h"
#include "../tora/pad_list.h"
#include <stdarg.h>
#include <assert.h>
#include <stdio.h>
#include <stddef.h>

using namespace tora;

int main() {
    std::string filename_("foo.tra");

    {
        std::shared_ptr<PadList>  p1 = std::make_shared<PadList>((int)1);
        MortalIntValue i5(5);
        p1->set(0, i5.get());
        SharedValue val = p1->get(0);
        ok(!!val.get());
        is(val->value_type, VALUE_TYPE_INT);
        is(val->to_int(), 5);

        SharedValue val1 = p1->get(1);
        is(val1->value_type, VALUE_TYPE_UNDEF);
    }

    printf("# second phase\n");
    {
        std::shared_ptr<PadList>  p1 = std::make_shared<PadList>(0);
        std::shared_ptr<PadList>  p2 = std::make_shared<PadList>(0, p1);
        std::shared_ptr<PadList>  p3 = std::make_shared<PadList>(0, p2);
        is((void*)p1->upper(1).get(), (void*)NULL);
        is(p2->upper(1).get(), p1.get());
        is(p3->upper(1).get(), p2.get());
        is(p3->upper(2).get(), p1.get());
        is((void*)p3->upper(3).get(), (void*)NULL);
    }

    printf("# third phase\n");

    {
        std::shared_ptr<PadList>  p1 = std::make_shared<PadList>(1);
        std::shared_ptr<PadList>  p2 = std::make_shared<PadList>(1, p1);
        std::shared_ptr<PadList>  p3 = std::make_shared<PadList>(3, p2);
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
