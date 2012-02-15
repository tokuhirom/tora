#include "range.h"

using namespace tora;

SharedPtr<StrValue> RangeValue::to_s() {
    SharedPtr<StrValue> v = new StrValue();
    std::ostringstream os;
    os << this->left->int_value;
    os << "..";
    os << this->right->int_value;
    v->set_str(os.str());
    return &(*(v));
}

