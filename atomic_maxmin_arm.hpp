/*
 * Implementations of max/min atomics on AArch64
 */

#ifndef __included_atomic_maxmin_arm
#define __included_atomic_maxmin_arm

#include <atomic>
#include <algorithm>
#include <cassert>

namespace atomic_maxmin_arm {

using namespace std;

#define DEF0(op, sz, rs) \
    if (m == memory_order_relaxed)                                          \
        __asm__ __volatile__(op sz " %" rs "1,%" rs "0,[%2]":"=r"(x):"r"(v),"r"(pv):);     \
    else if (m == memory_order_release)                                     \
        __asm__ __volatile__(op "l" sz " %" rs "1,%" rs "0,[%2]":"=r"(x):"r"(v),"r"(pv):);    \
    else if (m == memory_order_acquire || m == memory_order_consume)        \
        __asm__ __volatile__(op "a" sz " %" rs "1,%" rs "0,[%2]":"=r"(x):"r"(v),"r"(pv):);    \
    else if (m == memory_order_acq_rel || m == memory_order_seq_cst)        \
        __asm__ __volatile__(op "al" sz " %" rs "1,%" rs "0,[%2]":"=r"(x):"r"(v),"r"(pv):);   \
    else {                \
        assert(false);    \
    }

#define DEF1(op) \
    if (sizeof(T) == 8) {         \
        DEF0(op, "", "x")         \
    } else if (sizeof(T) == 4) {  \
        DEF0(op, "", "w")         \
    } else if (sizeof(T) == 2) {  \
        DEF0(op, "h", "w")        \
    } else if (sizeof(T) == 1) {  \
        DEF0(op, "b", "w")        \
    } else {                      \
        assert(false);            \
    }

#define DEF2(ls, op) \
    if (is_signed<T>::value) {    \
        DEF1(ls "s" op)           \
    } else {                      \
        DEF1(ls "u" op)           \
    }
    


/*
 * Preconditions for load() say that the order must not be release or acq_rel.
 */
constexpr
memory_order load_order(memory_order m)
{
    if (m == memory_order_release) {
        return memory_order_relaxed;
    } else if (m == memory_order_acq_rel) {
        return memory_order_acquire;
    } else {
        return m;
    }
}

template <typename T>
T atomic_fetch_max_explicit(atomic<T>* pv,
                            typename atomic<T>::value_type v,
                            memory_order m) noexcept {
    typename atomic<T>::value_type x;
    if (is_integral<T>::value) {
        assert(sizeof(T) == 8 || sizeof(T) == 4 || sizeof(T) == 2 || sizeof(T) == 1);
        DEF2("ld", "max")
    } else {
        x = pv->load(load_order(m));
        while (!pv->compare_exchange_weak(x, max(v, x), m, m));    
    }
    return x;
}


template <typename T>
T atomic_fetch_min_explicit(atomic<T>* pv,
                            typename atomic<T>::value_type v,
                            memory_order m) noexcept {
    typename atomic<T>::value_type x;
    if (is_integral<T>::value) {
        assert(sizeof(T) == 8 || sizeof(T) == 4 || sizeof(T) == 2 || sizeof(T) == 1);
        DEF2("ld", "min")
    } else {
        x = pv->load(load_order(m));
        while (!pv->compare_exchange_weak(x, min(v, x), m, m));    
    }
    return x;
}

#undef DEF2

} /* namespace */

using namespace atomic_maxmin_arm;

#endif /* included */

