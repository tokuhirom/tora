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
        /*
        for (int i = 0; i <= VAL()->NumberOfCapturingGroups(); i++) {
            if(res[i].data()) {
                std::cerr << " # " << std::string(res[i].data(), res[i].length()) << std::endl;
            } else {
                std::cerr << "# non match" << std::endl;
            }
        }
        */
        return tora::RE2_Regexp_Matched_new(vm, this, res);
    } else {
        return UndefValue::instance();
    }
}

