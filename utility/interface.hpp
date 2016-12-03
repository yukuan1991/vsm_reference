#pragma once

#include <exception>
#include <memory>
#include <type_traits>
#include <boost/iterator/counting_iterator.hpp>

#define MAYBE_UNUSED(var) (void)var

struct throw_reaction
{
    void operator () () { throw std::invalid_argument ("convert null pointer to not_null");}
};

template<typename T, typename NULL_REACTION = throw_reaction>
class not_null
{
public:
    template<typename U>
    not_null (U data) : not_null_data_ (data)
    {
        static_assert (std::is_pointer<T>::value, "not_null could only accept pointers");
        if (not_null_data_ == nullptr) NULL_REACTION()();
    }
    operator T () { return not_null_data_; }
    template<typename OTHER>
    not_null operator = (OTHER that) noexcept { not_null_data_ = that; }

    not_null operator = (const not_null& that) noexcept
    {
        not_null_data_ = that.not_null_data_;
        if (not_null_data_ == nullptr)
        {
            NULL_REACTION () ();
        }
    }

    T operator -> ()
    {
        return not_null_data_;
    }

    template<typename OTHER>
    not_null (const not_null<OTHER>& that) noexcept { not_null_data_ = that.get (); }
    T get () const noexcept {return not_null_data_; }
    ~not_null () = default;
private:
    T not_null_data_;
};


template<typename DATA, typename BINOP>
struct value_guard
{
    value_guard (DATA ref) :ref_ (ref)
    {
        if (!(BINOP () (ref_)))
        {
            throw std::logic_error ("cannot construct value_guard");
        }
    }

private:
    DATA ref_;
};

namespace std
{
template< class T >
constexpr bool is_function_v = is_function<T>::value;

template< class Base, class Derived >
constexpr bool is_base_of_v = is_base_of<Base, Derived>::value;

template< class T >
constexpr bool is_member_pointer_v = is_member_pointer<T>::value;
}

namespace detail {
template <class T>
struct is_reference_wrapper : std::false_type {};
template <class U>
struct is_reference_wrapper<std::reference_wrapper<U>> : std::true_type {};
template <class T>
constexpr bool is_reference_wrapper_v = is_reference_wrapper<T>::value;

template <class Base, class T, class Derived, class... Args>
auto INVOKE(T Base::*pmf, Derived&& ref, Args&&... args)
    noexcept(noexcept((std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...)))
 -> std::enable_if_t<std::is_function_v<T> &&
                     std::is_base_of_v<Base, std::decay_t<Derived>>,
    decltype((std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...))>
{
      return (std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...);
}

template <class Base, class T, class RefWrap, class... Args>
auto INVOKE(T Base::*pmf, RefWrap&& ref, Args&&... args)
    noexcept(noexcept((ref.get().*pmf)(std::forward<Args>(args)...)))
 -> std::enable_if_t<std::is_function_v<T> &&
                     is_reference_wrapper_v<std::decay_t<RefWrap>>,
    decltype((ref.get().*pmf)(std::forward<Args>(args)...))>

{
      return (ref.get().*pmf)(std::forward<Args>(args)...);
}

template <class Base, class T, class Pointer, class... Args>
auto INVOKE(T Base::*pmf, Pointer&& ptr, Args&&... args)
    noexcept(noexcept(((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...)))
 -> std::enable_if_t<std::is_function_v<T> &&
                     !is_reference_wrapper_v<std::decay_t<Pointer>> &&
                     !std::is_base_of_v<Base, std::decay_t<Pointer>>,
    decltype(((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...))>
{
      return ((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...);
}

template <class Base, class T, class Derived>
auto INVOKE(T Base::*pmd, Derived&& ref)
    noexcept(noexcept(std::forward<Derived>(ref).*pmd))
 -> std::enable_if_t<!std::is_function_v<T> &&
                     std::is_base_of_v<Base, std::decay_t<Derived>>,
    decltype(std::forward<Derived>(ref).*pmd)>
{
      return std::forward<Derived>(ref).*pmd;
}

template <class Base, class T, class RefWrap>
auto INVOKE(T Base::*pmd, RefWrap&& ref)
    noexcept(noexcept(ref.get().*pmd))
 -> std::enable_if_t<!std::is_function_v<T> &&
                     is_reference_wrapper_v<std::decay_t<RefWrap>>,
    decltype(ref.get().*pmd)>
{
      return ref.get().*pmd;
}

template <class Base, class T, class Pointer>
auto INVOKE(T Base::*pmd, Pointer&& ptr)
    noexcept(noexcept((*std::forward<Pointer>(ptr)).*pmd))
 -> std::enable_if_t<!std::is_function_v<T> &&
                     !is_reference_wrapper_v<std::decay_t<Pointer>> &&
                     !std::is_base_of_v<Base, std::decay_t<Pointer>>,
    decltype((*std::forward<Pointer>(ptr)).*pmd)>
{
      return (*std::forward<Pointer>(ptr)).*pmd;
}

template <class F, class... Args>
auto INVOKE(F&& f, Args&&... args)
    noexcept(noexcept(std::forward<F>(f)(std::forward<Args>(args)...)))
 -> std::enable_if_t<!std::is_member_pointer_v<std::decay_t<F>>,
    decltype(std::forward<F>(f)(std::forward<Args>(args)...))>
{
      return std::forward<F>(f)(std::forward<Args>(args)...);
}
} // namespace detail

template< class F, class... ArgTypes >
auto invoke(F&& f, ArgTypes&&... args)
    // exception specification for QoI
    noexcept(noexcept(detail::INVOKE(std::forward<F>(f), std::forward<ArgTypes>(args)...)))
 -> decltype(detail::INVOKE(std::forward<F>(f), std::forward<ArgTypes>(args)...))
{
    return detail::INVOKE(std::forward<F>(f), std::forward<ArgTypes>(args)...);
}

template<typename T>
auto weak (T&& t)
{
    std::weak_ptr<typename std::decay_t<T>::element_type> wp = t;
    return wp;
}

struct i_range
{
    boost::counting_iterator<int> start;
    boost::counting_iterator<int> stop;
    auto begin () { return start; }
    auto end () { return stop; }
};


inline auto step (int start, int stop)
{
    return i_range {start, stop};
}

inline auto step (int stop)
{
    return step (0, stop);
}

#define S1(x) #x
#define S2(x) S1(x)
#define LOCATION "   file: " __FILE__ " line: " S2(__LINE__)
#define except(x) \
    throw std::logic_error (x LOCATION)
