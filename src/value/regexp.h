#ifndef TORA_REGEXP_H_
#define TORA_REGEXP_H_

#include "../value.h"
#include <re2/re2.h>

namespace tora {

class AbstractRegexpValue : public Value {
public:
    AbstractRegexpValue() {
        this->value_type = VALUE_TYPE_REGEXP;
    }
    virtual ~AbstractRegexpValue() { }
    virtual bool ok() = 0;
    virtual const std::string& error() = 0;
    virtual const std::string& pattern() = 0;
    virtual bool match(std::string &str) = 0;
};

class RE2RegexpValue : public AbstractRegexpValue {
private:
    RE2 *re_value;
public:
    RE2RegexpValue(std::string &str) : AbstractRegexpValue() {
        re_value = new RE2(str);
    }
    ~RE2RegexpValue() {
        delete re_value;
    }
    bool ok() {
        return this->re_value->ok();
    }
    const std::string& pattern() {
        return this->re_value->pattern();
    }
    const std::string& error() {
        return this->re_value->error();
    }
    bool match(std::string &str) {
        return RE2::PartialMatch(str, this->re_value->pattern().c_str());
    }
    void dump(int indent) {
        print_indent(indent);
        printf("/%s/", re_value->pattern().c_str());
    }
    const char * type_str() { return "regexp"; }
};

};

#endif // TORA_REGEXP_H_
