#include "code.h"
#include "../vm.h"

using namespace tora;

CodeValue::~CodeValue() {
    if (code_params) {
        auto iter = code_params->begin();
        for (; iter!=code_params->end(); iter++) {
            delete *iter;
        }
        delete code_params;
    }

    if (is_native_) {
        delete callback_;
    }

    delete closure_var_names;
    delete closure_vars;
}

SharedPtr<StrValue> CodeValue::to_s() {
    return new StrValue("<code>"); // TODO
}

