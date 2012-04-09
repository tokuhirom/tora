#ifndef TORA_VALUE_FILE_PACKAGE_H_
#define TORA_VALUE_FILE_PACKAGE_H_

#include "value.h"

namespace tora {

class FilePackageImpl {
public:
    ID name_id_;
    boost::shared_ptr<std::map<ID, SharedPtr<Value>>> data_;
};

class FilePackageValue : public Value {
public:
    typedef std::map<ID, SharedPtr<Value>>::const_iterator const_iterator;

    FilePackageValue(ID name_id, const boost::shared_ptr<std::map<ID, SharedPtr<Value>>> & src) : Value(VALUE_TYPE_FILE_PACKAGE) {
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
