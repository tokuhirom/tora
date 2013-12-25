#include "regexp.h"
#include "array.h"
#include "../object.h"

using namespace tora;

RE2RegexpValue::RE2RegexpValue(std::string &str, int flags)
    : AbstractRegexpValue(flags) {
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
  ptr_value_ = (void *)new RE2(src, opt);
}

RE2RegexpValue::~RE2RegexpValue() { delete VAL(); }

SharedPtr<Value> RE2RegexpValue::match(VM *vm, const std::string &str) {
  std::shared_ptr<std::vector<re2::StringPiece>> res(
      new std::vector<re2::StringPiece>(VAL()->NumberOfCapturingGroups() + 1));

  if (VAL()->Match(str, 0, str.size(), RE2::UNANCHORED, res->data(),
                   VAL()->NumberOfCapturingGroups() + 1)) {
    return tora::RE2_Regexp_Matched_new(vm, this, res);
  } else {
    return new_undef_value();
  }
}

bool RE2RegexpValue::match_bool(VM *vm, const std::string &str) {
  std::vector<re2::StringPiece> res(VAL()->NumberOfCapturingGroups() + 1);

  if (VAL()->Match(str, 0, str.size(), RE2::UNANCHORED, res.data(),
                   VAL()->NumberOfCapturingGroups() + 1)) {
    return true;
  } else {
    return false;
  }
}

SharedPtr<Value> RE2RegexpValue::scan(VM *vm, const std::string &str) {
  std::vector<re2::StringPiece> buf(VAL()->NumberOfCapturingGroups() + 1);

  int start = 0;
  int end = str.size();
  MortalArrayValue res;

  while (VAL()->Match(str, start, end, RE2::UNANCHORED, buf.data(),
                      VAL()->NumberOfCapturingGroups() + 1)) {
    if (VAL()->NumberOfCapturingGroups() != 0) {
      MortalArrayValue av;
      for (int i = 1; i < VAL()->NumberOfCapturingGroups() + 1; i++) {
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

SharedPtr<Value> RE2RegexpValue::split(VM *vm, const std::string &str,
                                       int limit) {
  // $str.split(//)
  if (this->pattern() == "") {
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

    while (VAL()->Match(src, 0, src.length(), RE2::UNANCHORED, buf, 1)) {
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
