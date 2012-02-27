#ifndef TORA_PRIM_H_
#define TORA_PRIM_H_

#include <cassert>

class Prim {
protected:
    Prim(): refcnt(0) { }
    int refcnt;
    Prim(const Prim&) = delete;
    Prim& operator=(const Prim&) = delete;
public:
    virtual ~Prim() { }
    void release() {
        // assert(refcnt > 0);
        --refcnt;
        if (refcnt == 0) {
            delete this;
        }
    }
    void retain() {
        assert(refcnt >= 0);
        ++refcnt;
    }
};

#endif // TORA_PRIM_H_
