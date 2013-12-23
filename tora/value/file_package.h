#ifndef TORA_VALUE_FILE_PACKAGE_H_
#define TORA_VALUE_FILE_PACKAGE_H_

#include "../value.h"

namespace tora {

class FilePackageImpl {
public:
    ID name_id_;
    std::shared_ptr<std::map<ID, SharedPtr<tora::Value>>> data_;
};

class FilePackageValue : public tora::Value {
public:
    typedef std::map<ID, SharedPtr<tora::Value>>::const_iterator const_iterator;

    FilePackageValue(ID name_id, const std::shared_ptr<std::map<ID, SharedPtr<tora::Value>>> & src) : tora::Value(VALUE_TYPE_FILE_PACKAGE) {
        this->file_package_value_ = new FilePackageImpl();
        this->file_package_value_->name_id_ = name_id;
        this->file_package_value_->data_ = src;
    }
    ~FilePackageValue() {
        delete this->file_package_value_;
    }
    const_iterator find(ID id) const {
        return file_package_value_->data_->find(id);
    }
    const_iterator end() const {
        return file_package_value_->data_->end();
    }
};

}

#endif // TORA_VALUE_FILE_PACKAGE_H_
