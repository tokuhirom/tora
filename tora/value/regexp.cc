#include "regexp.h"
#include "array.h"
#include "../object.h"
#include <boost/shared_array.hpp>
#include <boost/scoped_array.hpp>

using namespace tora;

RE2RegexpValue::RE2RegexpValue(std::string &str, int flags) : AbstractRegexpValue(flags) {
    std::string src = str;
    RE2::Options opt;
    // allow Perl's \d \s \w \D \S \W
    opt.set_perl_classes(true);
    // allow Perl's \b \B (word boundary and not)
    opt.set_word_boundary(true);
    if (flags & REGEXP_IGNORECASE) {
        opt.set_case_sensitive(false);
    }
    if (flags & REGEXP_DOTALL) {
        src = "(?s)" + str;
    }
    if (flags & REGEXP_MULTILINE) {
        src = "(?m)" + str;
    }
    ptr_value_ = (void*)new RE2(src, opt);
}

RE2RegexpValue::~RE2RegexpValue() {
    delete VAL();
}

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
    boost::scoped_array<re2::StringPiece> buf(new re2::StringPiece[VAL()->NumberOfCapturingGroups() + 1]);

    int start = 0;
    int end = str.size();
    SharedPtr<ArrayValue> res = new ArrayValue();

    while (VAL()->Match(str, start, end, RE2::UNANCHORED, buf.get(), VAL()->NumberOfCapturingGroups()+1)) {
        if (VAL()->NumberOfCapturingGroups() != 0) {
            SharedPtr<ArrayValue> av = new ArrayValue();
            for (int i=1; i< VAL()->NumberOfCapturingGroups()+1; i++) {
                if (buf[i].data()) {
                    av->push_back(new StrValue(buf[i].as_string()));
                } else {
                    av->push_back(UndefValue::instance());
                }
            }
            res->push_back(av);
        } else {
            res->push_back(new StrValue(buf[0].as_string()));
        }
        start = buf[0].data() - str.c_str() + (buf[0].length() > 0 ? buf[0].length() : 1);
    }
    return res;
}

SharedPtr<Value> RE2RegexpValue::split(VM *vm, const std::string &str, int limit) {
    // $str.split(//)
    if (this->pattern() == "") {
        SharedPtr<ArrayValue> res = new ArrayValue();
        size_t i = 0;
        for (; i<str.size(); i++) {
            if (limit==0 || res->size()+1 < limit) {
                res->push_back(new StrValue(str.substr(i, 1)));
            } else {
                res->push_back(new StrValue(str.substr(i)));
                break;
            }
        }
        return res;
    } else {
        re2::StringPiece buf[1];

        int start = 0;
        SharedPtr<ArrayValue> res = new ArrayValue();

        std::string src = str;

        while (VAL()->Match(src, 0, src.length(), RE2::UNANCHORED, buf, 1)) {
            if (limit==0 || res->size()+1 < limit) {
                res->push_back(new StrValue(src.substr(start, buf[0].data() - src.c_str())));
            } else {
                break;
            }
            src = src.substr((buf[0].data()-src.c_str())+buf[0].length());
        }
        if (src.size() > 0) {
            res->push_back(new StrValue(src));
        }
        return res;
    }
}

