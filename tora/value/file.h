#ifndef TORA_FILE_H_
#define TORA_FILE_H_

#include "../value.h"
#include <cstdio>

namespace tora {

class FileValue : public Value {
private:
    FILE *VAL() const {
        return boost::get<FILE*>(this->value_);
    }
public:
    FileValue() : Value(VALUE_TYPE_FILE) {
        this->value_ = (FILE*)NULL;
    }
    ~FileValue() {
        if (VAL()) { this->close(); }
    }
    void close() {
        if (VAL()) { fclose(this->VAL()); this->value_ = (FILE*)NULL; }
    }
    bool open(std::string &fname, std::string &mode) {
        this->value_ = fopen(fname.c_str(), mode.c_str());
        return !!VAL();
    }
    FILE *fp() {
        return VAL();
    }
    SharedPtr<StrValue> read() {
        // slurp
        std::string s;
        const int bufsiz = 4096;
        char buffer[bufsiz];
        size_t n;
        while ((n = fread(buffer, sizeof(char), bufsiz, VAL())) != 0) {
            s.append(buffer, n);
        }
        if (feof(VAL())) {
            return new StrValue(s);
        } else {
            // err?
            throw new ErrnoExceptionValue(errno);
        }
    }
};

};

#endif // TORA_FILE_H_
