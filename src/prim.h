#ifndef TORA_PRIM_H_
#define TORA_PRIM_H_

class Prim {
protected:
    Prim(): refcnt(0) { }
    int refcnt;
public:
    virtual ~Prim() { }
    void release() {
        --refcnt;
        if (refcnt == 0) {
            delete this;
        }
    }
    void retain() {
        ++refcnt;
    }
};

#endif // TORA_PRIM_H_
