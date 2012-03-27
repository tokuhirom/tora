#include "regexp.h"
#include "array.h"
#include "../object.h"
#include <boost/shared_array.hpp>

using namespace tora;

SharedPtr<Value> RE2RegexpValue::match(VM *vm, const std::string &str) {
    boost::shared_array<re2::StringPiece> res(new re2::StringPiece[VAL()->NumberOfCapturingGroups() + 1]);

    if (VAL()->Match(
        str, 0, str.size(), RE2::UNANCHORED, res.get(), VAL()->NumberOfCapturingGroups()+1
        )) {
        return tora::RE2_Regexp_Matched_new(vm, this, res);
    } else {
        return UndefValue::instance();
    }
}

SharedPtr<Value> RE2RegexpValue::scan(VM *vm, const std::string &str) {
    re2::StringPiece buf[1];
    int start = 0;
    int end = str.size();
    SharedPtr<ArrayValue> res = new ArrayValue();

    while (VAL()->Match(str, start, end, RE2::UNANCHORED, buf, 1)) {
        res->push_back(new StrValue(buf[0].as_string()));
        start = buf[0].data() - str.c_str() + 1;
    }
    return res;
}

