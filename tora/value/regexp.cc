#include "regexp.h"
#include "array.h"
#include "../object.h"
#include "../vm.h"
#include "../symbols.gen.h"
#include "../value/object.h"
#include "../value/pointer.h"
#include <vector>
#include <memory>

using namespace tora;

namespace tora {
  struct RegexpImpl {
    RE2 re2;
    int flags;
    RegexpImpl(std::string src, RE2::Options &opt, int _flags)
      : re2(src, opt), flags(_flags) { }
  };

  struct RegexpMatchedImpl {
    SharedValue re;
    std::shared_ptr<std::vector<re2::StringPiece>> matches;

    RegexpMatchedImpl(
        Value* _re,
        const std::shared_ptr<std::vector<re2::StringPiece>>& _matches)
        : re(_re), matches(_matches) {}
  };
}

static RegexpImpl* regexp(Value* self)
{
  return static_cast<RegexpImpl*>(self->ptr_value_);
}

static RE2* get_re2(Value* self)
{
  return &(regexp(self)->re2);
}

static RegexpMatchedImpl* matched(Value* self)
{
  return static_cast<RegexpMatchedImpl*>(
      get_ptr_value(object_data(self)));
}

bool tora::regexp_ok(Value * self)
{
  return get_re2(self)->ok();
}

std::string tora::regexp_pattern(Value * self)
{
  return get_re2(self)->pattern();
}

std::string tora::regexp_error(Value * self)
{
  return get_re2(self)->error();
}

int tora::regexp_get_flags(Value* self)
{
  return regexp(self)->flags;
}

SharedValue tora::regexp_matches_get_item(Value* self, tra_int i)
{
  const re2::StringPiece& res = matched(self)->matches->at(i);
  if (res.data()) {
    MortalStrValue s(std::string(res.data(), res.length()));
    return s.get();
  } else {
    MortalUndefValue u;
    return u.get();
  }
}

int tora::regexp_number_of_capturing_groups(Value *self)
{
  return get_re2(self)->NumberOfCapturingGroups();
}


void tora::regexp_dump(Value* self, int indent)
{
  print_indent(indent);
  printf("/%s/", get_re2(self)->pattern().c_str());
}

std::string tora::regexp_replace(Value* self, const std::string &str, const std::string &rewrite, int &replacements)
{
  // optimizable
  std::string buf(str);
  if (regexp(self)->flags & REGEXP_GLOBAL) {
    replacements = RE2::GlobalReplace(&buf, *get_re2(self), rewrite);
  } else {
    replacements = RE2::Replace(&buf, *get_re2(self), rewrite);
  }
  return buf;
}

std::string tora::regexp_quotemeta(const std::string &str)
{
  return RE2::QuoteMeta(str);
}

void tora::regexp_free(Value* self)
{
  delete regexp(self);
}

Value* MortalRegexpValue::new_value(std::string str, int flags)
{
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
  return new Value(VALUE_TYPE_REGEXP, static_cast<void *>(new RegexpImpl(src, opt, flags)));
}


SharedValue tora::regexp_match(Value* self, VM *vm, const std::string &str) {
  std::shared_ptr<std::vector<re2::StringPiece>> res = std::make_shared<std::vector<re2::StringPiece>>(
      get_re2(self)->NumberOfCapturingGroups() + 1
  );

  if (get_re2(self)->Match(str, 0, str.size(), RE2::UNANCHORED, res->data(),
                   get_re2(self)->NumberOfCapturingGroups() + 1)) {
    MortalPointerValue p(new RegexpMatchedImpl(self, res));
    MortalObjectValue o(
      vm,
      vm->get_builtin_class(SYMBOL_RE2_REGEXP_MATCHED_CLASS).get(),
      p.get()
    );
    return o.get();
  } else {
    return new_undef_value();
  }
}

SharedValue tora::regexp_matched_regexp(Value* self)
{
  return matched(self)->re;
}

void tora::regexp_matched_free(Value* self)
{
  delete matched(self);
}

bool tora::regexp_match_bool(Value* self, const std::string &str) {
  std::vector<re2::StringPiece> res(get_re2(self)->NumberOfCapturingGroups() + 1);

  if (get_re2(self)->Match(str, 0, str.size(), RE2::UNANCHORED, res.data(),
                   get_re2(self)->NumberOfCapturingGroups() + 1)) {
    return true;
  } else {
    return false;
  }
}

SharedValue tora::regexp_scan(Value* self, VM *vm, const std::string &str)
{
  std::vector<re2::StringPiece> buf(get_re2(self)->NumberOfCapturingGroups() + 1);

  int start = 0;
  int end = str.size();
  MortalArrayValue res;

  while (get_re2(self)->Match(str, start, end, RE2::UNANCHORED, buf.data(),
                      get_re2(self)->NumberOfCapturingGroups() + 1)) {
    if (get_re2(self)->NumberOfCapturingGroups() != 0) {
      MortalArrayValue av;
      for (int i = 1; i < get_re2(self)->NumberOfCapturingGroups() + 1; i++) {
        if (buf[i].data()) {
          MortalStrValue s(buf[i].as_string());
          array_push_back(av.get(), s.get());
        } else {
          array_push_back(av.get(), new_undef_value());
        }
      }
      array_push_back(res.get(), av.get());
    } else {
      MortalStrValue s(buf[0].as_string());
      array_push_back(res.get(), s.get());
    }
    start = buf[0].data() - str.c_str() +
            (buf[0].length() > 0 ? buf[0].length() : 1);
  }
  return res.get();
}

SharedValue tora::regexp_split(Value* self, VM *vm, const std::string &str, int limit)
{
  // $str.split(//)
  if (get_re2(self)->pattern() == "") {
    MortalArrayValue res;
    size_t i = 0;
    for (; i < str.size(); i++) {
      if (limit == 0 || array_size(res.get()) + 1 < limit) {
        MortalStrValue s(str.substr(i, 1));
        array_push_back(res.get(), s.get());
      } else {
        MortalStrValue s(str.substr(i));
        array_push_back(res.get(), s.get());
        break;
      }
    }
    return res.get();
  } else {
    re2::StringPiece buf[1];

    int start = 0;
    MortalArrayValue res;

    std::string src = str;

    while (get_re2(self)->Match(src, 0, src.length(), RE2::UNANCHORED, buf, 1)) {
      if (limit == 0 || array_size(res.get()) + 1 < limit) {
        MortalStrValue s(src.substr(start, buf[0].data() - src.c_str()));
        array_push_back(res.get(), s.get());
      } else {
        break;
      }
      src = src.substr((buf[0].data() - src.c_str()) + buf[0].length());
    }
    if (src.size() > 0) {
      MortalStrValue s(src);
      array_push_back(res.get(), s.get());
    }
    return res.get();
  }
}
