#ifndef TORA_REGEXP_H_
#define TORA_REGEXP_H_

#include "../value.h"
#include <re2/re2.h>

namespace tora {

enum regexp_flags {
    REGEXP_GLOBAL     = 1,
    REGEXP_MULTILINE  = 2,
    REGEXP_IGNORECASE = 4,
    REGEXP_EXPANDED   = 8
};

class AbstractRegexpValue : public Value {
protected:
    int flags_;
public:
    AbstractRegexpValue(int flags) : Value(VALUE_TYPE_REGEXP), flags_(flags) { }
    virtual ~AbstractRegexpValue() { }
    virtual bool ok() = 0;
    virtual const std::string& error() = 0;
    virtual const std::string& pattern() = 0;
    virtual bool match(std::string &str) = 0;
    virtual int flags() {
        return flags_;
    }
};

class RE2RegexpValue : public AbstractRegexpValue {
private:
    RE2 *re_value;
public:
    RE2RegexpValue(std::string &str, int flags) : AbstractRegexpValue(flags) {
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
};

static inline int regexp_flag(char c) {
    switch (c) {
    case 'm':
        return REGEXP_MULTILINE;
    case 'g':
        return REGEXP_GLOBAL;
    case 'i':
        return REGEXP_IGNORECASE;
    case 'x':
        return REGEXP_EXPANDED;
    default:
        printf("[BUG] Unknown regexp option: %c\n", c);
        abort();
    }
}

};

#endif // TORA_REGEXP_H_
