﻿#pragma once
#include <thread>
#include "hash.h"
#include "hook.h"
template <typename T, int off>
inline T& dAccess(void* ptr) {
    return *(T*)(((uintptr_t)ptr) + off);
}
template <typename T, int off>
inline T const& dAccess(void const* ptr) {
    return *(T*)(((uintptr_t)ptr) + off);
}
template <typename T>
inline T& dAccess(void* ptr, uintptr_t off) {
    return *(T*)(((uintptr_t)ptr) + off);
}
template <typename T>
inline const T& dAccess(void const* ptr, uintptr_t off) {
    return *(T*)(((uintptr_t)ptr) + off);
}
#define __WEAK __declspec(selectany)
//#define GetServerSymbol(x) dlsym_real(x)
template <CHash, CHash>
__WEAK void* __ptr_cache;
template <CHash hash, CHash hash2>
inline static void* dlsym_cache(const char* fn) {
    if (!__ptr_cache<hash, hash2>) {
        __ptr_cache<hash, hash2> = dlsym_real(fn);
        if (!__ptr_cache<hash, hash2>) {
            printf("Cant found sym %s\n", fn);
            std::this_thread::sleep_for(std::chrono::seconds(10));
            exit(1);
        }
    }
    return __ptr_cache<hash, hash2>;
}
#define VA_EXPAND(...) __VA_ARGS__
//#define Call(fn, ret, ...) __CALL_IMP<do_hash(fn), ret, __VA_ARGS__>(fn)
//#define SymCall(fn, ret, ...) (__imp_Call<ret, __VA_ARGS__>(fn))
#define SymCall(fn, ret, ...) ((ret(*)(__VA_ARGS__))(dlsym_real(fn)))
#define SYM(fn) (dlsym_real(fn))
#define dlsym(xx) SYM(xx)

class THookRegister {
public:
    THookRegister(void* address, void* hook, void** org) {
        auto ret = HookFunction(address, org, hook);
        if (ret != 0) {
            printf("FailedToHook: %p\n", address);
        }
    }
    THookRegister(char const* sym, void* hook, void** org) {
        auto found = dlsym_real(sym);
        if (found == nullptr) {
            printf("FailedToHook: %p\n", sym);
        }
        auto ret = HookFunction(found, org, hook);
        if (ret != 0) {
            printf("FailedToHook: %s\n", sym);
        }
    }
    template <typename T>
    THookRegister(const char* sym, T hook, void** org) {
        union
        {
            T a;
            void* b;
        } hookUnion;
        hookUnion.a = hook;
        THookRegister(sym, hookUnion.b, org);
    }
    template <typename T>
    THookRegister(void* sym, T hook, void** org) {
        union
        {
            T a;
            void* b;
        } hookUnion;
        hookUnion.a = hook;
        THookRegister(sym, hookUnion.b, org);
    }
};
#define VA_EXPAND(...) __VA_ARGS__
template <CHash, CHash>
struct THookTemplate;
template <CHash, CHash>
extern THookRegister THookRegisterTemplate;

#define _TInstanceHook(class_inh, pclass, iname, sym, ret, ...)                               \
    template <>                                                                               \
    struct THookTemplate<do_hash(iname), do_hash2(iname)> class_inh {                         \
        typedef ret (THookTemplate::*original_type)(__VA_ARGS__);                             \
        static original_type &_original() {                                                   \
            static original_type storage;                                                     \
            return storage;                                                                   \
        }                                                                                     \
        template <typename... Params>                                                         \
        static ret original(pclass *_this, Params &&... params) {                             \
            return (((THookTemplate *)_this)->*_original())(std::forward<Params>(params)...); \
        }                                                                                     \
        ret _hook(__VA_ARGS__);                                                               \
    };                                                                                        \
    template <>                                                                               \
    static THookRegister THookRegisterTemplate<do_hash(iname), do_hash2(iname)>{              \
        sym, &THookTemplate<do_hash(iname), do_hash2(iname)>::_hook,                          \
        (void **)&THookTemplate<do_hash(iname), do_hash2(iname)>::_original()};               \
    ret THookTemplate<do_hash(iname), do_hash2(iname)>::_hook(__VA_ARGS__)

#define _TInstanceDefHook(iname, sym, ret, type, ...) \
    _TInstanceHook( : public type, type, iname, sym, ret, VA_EXPAND(__VA_ARGS__))
#define _TInstanceNoDefHook(iname, sym, ret, ...) \
    _TInstanceHook(, void, iname, sym, ret, VA_EXPAND(__VA_ARGS__))

#define _TStaticHook(pclass, iname, sym, ret, ...)                               \
    template <>                                                                  \
    struct THookTemplate<do_hash(iname), do_hash2(iname)> pclass {               \
        typedef ret (*original_type)(__VA_ARGS__);                               \
        static original_type &_original() {                                      \
            static original_type storage;                                        \
            return storage;                                                      \
        }                                                                        \
        template <typename... Params>                                            \
        static ret original(Params &&... params) {                               \
            return _original()(std::forward<Params>(params)...);                 \
        }                                                                        \
        static ret _hook(__VA_ARGS__);                                           \
    };                                                                           \
    template <>                                                                  \
    static THookRegister THookRegisterTemplate<do_hash(iname), do_hash2(iname)>{ \
        sym, &THookTemplate<do_hash(iname), do_hash2(iname)>::_hook,             \
        (void **)&THookTemplate<do_hash(iname), do_hash2(iname)>::_original()};  \
    ret THookTemplate<do_hash(iname), do_hash2(iname)>::_hook(__VA_ARGS__)

#define _TStaticDefHook(iname, sym, ret, type, ...) \
    _TStaticHook( : public type, iname, sym, ret, VA_EXPAND(__VA_ARGS__))
#define _TStaticNoDefHook(iname, sym, ret, ...) \
    _TStaticHook(, iname, sym, ret, VA_EXPAND(__VA_ARGS__))

#define THook2(iname, ret, sym, ...) _TStaticNoDefHook(iname, sym, ret, VA_EXPAND(__VA_ARGS__))
#define THook(ret, sym, ...) THook2(sym, ret, sym, VA_EXPAND(__VA_ARGS__))
#define TStaticHook2(iname, ret, sym, type, ...) \
    _TStaticDefHook(iname, sym, ret, type, VA_EXPAND(__VA_ARGS__))
#define TStaticHook(ret, sym, type, ...) TStaticHook2(sym, ret, sym, type, VA_EXPAND(__VA_ARGS__))
#define TClasslessInstanceHook2(iname, ret, sym, ...) \
    _TInstanceNoDefHook(iname, sym, ret, VA_EXPAND(__VA_ARGS__))
#define TClasslessInstanceHook(ret, sym, ...) \
    TClasslessInstanceHook2(sym, ret, sym, VA_EXPAND(__VA_ARGS__))
#define TInstanceHook2(iname, ret, sym, type, ...) \
    _TInstanceDefHook(iname, sym, ret, type, VA_EXPAND(__VA_ARGS__))
#define TInstanceHook(ret, sym, type, ...) \
    TInstanceHook2(sym, ret, sym, type, VA_EXPAND(__VA_ARGS__))


template<int len> struct PatchHelper {
    unsigned char data[len];
    using ref_t = char(&)[len];
    constexpr bool operator==(ref_t ref) const noexcept {
        return memcmp(data, ref, sizeof data) == 0;
    }
    constexpr bool operator!=(ref_t ref) const noexcept {
        return memcmp(data, ref, sizeof data) != 0;
    }
    constexpr bool operator==(PatchHelper ref) const noexcept {
        return memcmp(data, ref.data, sizeof data) == 0;
    }
    constexpr bool operator!=(PatchHelper ref) const noexcept {
        return memcmp(data, ref.data, sizeof data) != 0;
    }
    void operator=(ref_t ref) { memcpy(data, ref, sizeof data); }
    void DoPatch(PatchHelper expected, PatchHelper patched);
    void EasyPatch(PatchHelper expected, PatchHelper patched);

    std::string Dump() const noexcept {
        char buffer[2 * len + 1] = {};
        char* ptr = buffer;
        for (auto ch : data)
            ptr += sprintf(ptr, "%02X", (unsigned)ch);
        return { buffer };
    }
};

struct NopFiller {
    template <int len>
    operator PatchHelper<len>() {
        PatchHelper<len> ret;
        memset(ret.data, 0x90, len);
        return ret;
    }
};

struct FailedToPatch : std::exception {
    std::string info;
    template <int len>
    FailedToPatch(PatchHelper<len> const& current, PatchHelper<len> const& expected) {
        info = "Failed to patch: expected " + expected.Dump() + ", but actual " + current.Dump();
    }
    const char* what() const noexcept { return info.c_str(); }
};

template <int len>
void PatchHelper<len>::DoPatch(PatchHelper<len> expected, PatchHelper<len> patched) {
    if (*this == expected)
        *this = patched;
    else
        throw FailedToPatch(*this, expected);
}

template <int len>
void PatchHelper<len>::EasyPatch(PatchHelper<len> expected, PatchHelper<len> patched) {
    DWORD old, tmp;
    VirtualProtect((LPVOID)this, (SIZE_T)len, PAGE_EXECUTE_READWRITE, &old);
    DoPatch(expected, patched);
    VirtualProtect((LPVOID)this, (SIZE_T)len, old, &tmp);
}