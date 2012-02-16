#ifndef TORA_FILE_H_
#define TORA_FILE_H_

#include "../value.h"
#include <cstdio>

namespace tora {

class FileValue : public Value {
private:
    FILE *fp;
public:
    FileValue() : fp(NULL) {
        value_type = VALUE_TYPE_FILE;
    }
    ~FileValue() {
        if (fp) { this->close(); }
    }
    void close() {
        if (fp) { fclose(this->fp); }
    }
    bool open(std::string &fname, std::string &mode) {
        this->fp = fopen(fname.c_str(), mode.c_str());
        return !!this->fp;
    }
    SharedPtr<StrValue> read() {
        // slurp
        std::string s;
        const int bufsiz = 4096;
        char buffer[bufsiz];
        size_t n;
        while ((n = fread(buffer, sizeof(char), bufsiz, this->fp)) != 0) {
            s.append(buffer, n);
        }
        if (feof(this->fp)) {
            return new StrValue(s);
        } else {
            // err?
            // TODO exception
            abort();
        }
    }
    void dump(int indent) {
        print_indent(indent);
        printf("[dump] #<FILE>\n");
    }
    const char *type_str() { return "file"; }
};

};

#endif // TORA_FILE_H_
