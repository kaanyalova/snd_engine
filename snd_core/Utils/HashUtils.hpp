#include <cstddef>
#include <functional>
class HashUtils {
  public:
    template <class T>
    static inline void hash_combine(size_t& s, const T& v) {
        std::hash<T> h;
        s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
    }
};