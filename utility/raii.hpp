#pragma once

#include <utility>
#include <memory>
#include "folly/detail/UncaughtExceptionCounter.h"

template<typename T> decltype (auto) move (T& param)
{
    static_assert (!std::is_const<T>::value, "不能对常量使用move");

    using return_type = typename std::remove_reference<T>::type;
    return static_cast<return_type&&> (param);
}

template<typename T> decltype (auto) move (T&& param)
{
    static_assert (std::is_rvalue_reference<T>::value, "右值无需move");

    using return_type = typename std::remove_reference<T>::type;
    return static_cast<return_type&&> (param);
}

enum class __scope_guard_on_exit__ {};
enum class __scope_guard_on_fail__ {};

template<typename CALLABLE>
struct __scope_guard__
{
    CALLABLE c_;
    __scope_guard__ (CALLABLE&& c) : c_ (std::forward<CALLABLE> (c)) {}
    ~__scope_guard__ ()
    {
        c_ ();
    }
};

template <typename CALLABLE, bool exec_on_exception>
class __scope_guard_for_exception__
{
public:
    explicit __scope_guard_for_exception__ (CALLABLE&& c) : c_ (std::forward<CALLABLE> (c)) {}
    explicit __scope_guard_for_exception__ (const CALLABLE& c) : c_ (c) {}
    __scope_guard_for_exception__ (__scope_guard_for_exception__&& other) : c_ (::move (other.c_)),
        exception_counter_ (::move (other.exception_counter_)) {  }

    ~__scope_guard_for_exception__ ()
    {
        if (exec_on_exception == exception_counter_.isNewUncaughtException ())
        {
            c_ ();
        }
    }

private:
    CALLABLE c_;
    folly::detail::UncaughtExceptionCounter exception_counter_;

    void* operator new (std::size_t) = delete;
};

template<typename fun>
__scope_guard__<fun> operator + (__scope_guard_on_exit__, fun&& fn)
{
    return __scope_guard__<fun> (std::forward<fun> (fn));
}

template <typename fun>
__scope_guard_for_exception__<typename std::decay_t<fun>, true>
operator + (__scope_guard_on_fail__, fun&& f)
{
    return __scope_guard_for_exception__<typename std::decay_t<fun>, true> (std::forward<fun> (f));
}

#define __CONCAT_IMPL__(str1,str2) str1##str2
#define CONCAT(str1,str2) __CONCAT_IMPL__(str1,str2)

#define __ANONYMOUS_VAR__(str) CONCAT(str,__LINE__)

/// 当退出作用域时调用 使用格式为 SCOPE_EXIT { fclose (fp);};
#define SCOPE_EXIT \
    auto __ANONYMOUS_VAR__(HELPER_SCOPE_EXIT_STATE) \
    = __scope_guard_on_exit__ () + [&]()

#define SCOPE_FAIL \
    auto __ANONYMOUS_VAR__(SCOPE_FAIL_STATE) \
    = __scope_guard_on_fail__ () + [&] ()



/// unique_ptr utility

template<typename DATA, DATA null_val = DATA {}>
struct ptr_simulator
{
    constexpr ptr_simulator (DATA data) noexcept : data_ (data)
    {
        static_assert (std::is_pod<DATA>::value, "ptr_simulator only supports POD data");
    }
    constexpr ptr_simulator (std::nullptr_t = nullptr) noexcept :ptr_simulator (null_val) {}
    operator DATA () noexcept { return data_; }
    explicit operator bool  () noexcept { return data_ != null_val; }
    bool operator == (ptr_simulator that) noexcept { return data_ == that.data_; }
    bool operator != (ptr_simulator that) noexcept { return data_ != that.data_; }
    constexpr DATA data () noexcept { return data_; }
private:
    DATA data_;
};

template<typename simulator>
using wrap_closer = std::unique_ptr<void, simulator>;

/// 比 std 中更加严格的move 不会让常量move通过编译


