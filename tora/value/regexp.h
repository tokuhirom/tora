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
    virtual SharedPtr<Value> match(VM *vm, const std::string &str) = 0;
    virtual std::string replace(const std::string &str, const std::string &rewrite, int &replacements) const = 0;
    virtual int flags() const {
        return flags_;
    }
};

class RE2RegexpValue : public AbstractRegexpValue {
private:
    RE2* VAL() const {
        return static_cast<RE2*>(boost::get<void*>(this->value_));
    }
public:
    RE2RegexpValue(std::string &str, int flags) : AbstractRegexpValue(flags) {
        RE2::Options opt;
        // allow Perl's \d \s \w \D \S \W
        opt.set_perl_classes(true);
        // allow Perl's \b \B (word boundary and not)
        opt.set_word_boundary(true);
        if (flags & REGEXP_IGNORECASE) {
            opt.set_case_sensitive(false);
        }
        value_ = (void*)new RE2(str, opt);
    }
    ~RE2RegexpValue() {
        delete VAL();
    }
    bool ok() {
        return VAL()->ok();
    }
    const std::string& pattern() {
        return VAL()->pattern();
    }
    const std::string& error() {
        return VAL()->error();
    }
    SharedPtr<Value> match(VM *vm, const std::string &str);
    static std::string quotemeta(const std::string &str) {
        return RE2::QuoteMeta(str);
    }
    std::string replace(const std::string &str, const std::string &rewrite, int &replacements) const {
        // optimizable
        std::string buf(str);
        if (this->flags() & REGEXP_GLOBAL) {
            replacements = RE2::GlobalReplace(&buf, *(VAL()), rewrite);
        } else {
            replacements = RE2::Replace(&buf, *(VAL()), rewrite);
        }
        return buf;
    }
    void dump(int indent) {
        print_indent(indent);
        printf("/%s/", VAL()->pattern().c_str());
    }
    int number_of_capturing_groups() const {
        return VAL()->NumberOfCapturingGroups();
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
