#ifndef TORA_REGEXP_H_
#define TORA_REGEXP_H_

#include "../value.h"

namespace tora {

  // ref. http://docs.python.org/library/re.html#module-contents
  enum regexp_flags {
    REGEXP_GLOBAL = 1,      // 'g'
    REGEXP_MULTILINE = 2,   // 'm'
    REGEXP_IGNORECASE = 4,  // 'i'
    REGEXP_EXPANDED = 8,    // 'x'
    REGEXP_DOTALL = 16      // 's'
  };

  class MortalRegexpValue : public MortalValue {
    static Value* new_value(std::string str, int flags);
  public:
    MortalRegexpValue(std::string str, int flags)
      : MortalValue(new_value(str, flags)) { }
  };


  SharedValue regexp_match(Value * self, VM *vm, const std::string &str);
  std::string regexp_pattern(Value * self);
  std::string regexp_error(Value * self);
  bool regexp_ok(Value * self);
  SharedValue regexp_scan(Value* self, VM *vm, const std::string &str);
  SharedValue regexp_split(Value* self, VM *vm, const std::string &str, int limit);
  void regexp_dump(Value* self, int indent);
  int regexp_number_of_capturing_groups(Value *self);
  std::string regexp_quotemeta(const std::string &str);
  void regexp_free(Value* self);
  int regexp_get_flags(Value* self);
  bool regexp_match_bool(Value* self, const std::string &str);
  std::string regexp_replace(Value* self, const std::string &str, const std::string &rewrite, int &replacements);
  SharedValue regexp_matches_get_item(Value* self, tra_int i);
  void regexp_matched_free(Value* self);

  SharedValue regexp_matched_regexp(Value* self);

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
    case 's':
      return REGEXP_DOTALL;
    default:
      printf("[BUG] Unknown regexp option: %c\n", c);
      abort();
  }
}
};

#endif  // TORA_REGEXP_H_
