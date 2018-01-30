#ifndef NonCopyable_h
#define NonCopyable_h

class NonCopyable {
protected:
    NonCopyable() = default;
private:
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};
#endif /* NonCopyable_h */