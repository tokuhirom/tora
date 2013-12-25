#ifndef TORA_FILE_H_
#define TORA_FILE_H_

#include "../value.h"
#include <cstdio>

namespace tora {

class FileValue : public Value {
private:
    FILE *VAL() const {
        return (this->file_value_);
    }
public:
    FileValue() : Value(VALUE_TYPE_FILE) {
        this->file_value_ = (FILE*)NULL;
    }
    explicit FileValue(FILE *fp) : Value(VALUE_TYPE_FILE) {
        this->file_value_ = fp;
    }
    ~FileValue() {
        if (VAL()) { this->close(); }
    }
    void close() {
        if (VAL()) { fclose(this->VAL()); this->file_value_ = (FILE*)NULL; }
    }
    bool open(const std::string &fname, const std::string &mode) {
        this->file_value_ = fopen(fname.c_str(), mode.c_str());
        return !!VAL();
    }
    FILE *fp() {
        return VAL();
    }
    std::string read() {
        // slurp
        std::string s;
        const int bufsiz = 4096;
        char buffer[bufsiz];
        size_t n;
        while ((n = fread(buffer, sizeof(char), bufsiz, VAL())) != 0) {
            s.append(buffer, n);
        }
        if (feof(VAL())) {
            return s;
        } else {
            // err?
            throw new ErrnoExceptionValue(tora::get_errno());
        }
    }
};

};

#endif // TORA_FILE_H_
