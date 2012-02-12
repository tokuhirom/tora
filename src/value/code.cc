#include "code.h"

using namespace tora;

SharedPtr<StrValue> CodeValue::to_s() {
    return new StrValue("<code>"); // TODO
}

