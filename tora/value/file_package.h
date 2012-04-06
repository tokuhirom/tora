#ifndef TORA_VALUE_FILE_PACKAGE_H_
#define TORA_VALUE_FILE_PACKAGE_H_

#include "value.h"

namespace tora {

class FilePackageValue : public Value {
public:
    FilePackageValue(const boost::shared_ptr<std::map<ID, SharedPtr<Value>>> & src) : Value(VALUE_TYPE_FILE_PACKAGE) {
        this->file_package_value_ = new boost::shared_ptr<std::map<ID, SharedPtr<Value>>>(src);
    }
    ~FilePackageValue() {
        delete this->file_package_value_;
    }
};

}

#endif // TORA_VALUE_FILE_PACKAGE_H_
