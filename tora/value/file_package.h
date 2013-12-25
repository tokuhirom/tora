#ifndef TORA_VALUE_FILE_PACKAGE_H_
#define TORA_VALUE_FILE_PACKAGE_H_

#include "../value.h"
#include <map>

namespace tora {

class MortalFilePackageValue : public MortalValue {
  static Value* new_file_package_value(ID name_id, const std::shared_ptr<std::map<ID, SharedValue>> & src);
public:
  MortalFilePackageValue(ID name_id, const std::shared_ptr<std::map<ID, SharedValue>> & src)
    : MortalValue(new_file_package_value(name_id, src)) { }
};

Value* file_package_find(Value * v, ID id);

}

#endif // TORA_VALUE_FILE_PACKAGE_H_
