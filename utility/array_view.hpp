#pragma once

#include <string>
#include <vector>
#include <exception>
#include <array>
#include <assert.h>


template<typename DATA>
class array_view
{
public:
    using value_type				=				DATA;
    using pointer					=				value_type*;
    using const_pointer				=				const pointer;
    using iterator					=				DATA*;
    using const_iterator			=				const iterator;
    using reference					=		  		value_type&;
    using const_reference			=				value_type&;
    using reverse_iterator			=				std::reverse_iterator<iterator>;
    using const_reverse_iterator	=				std::reverse_iterator<const_iterator>;
    using size_type					=				uint32_t;
    using difference_type			=				ptrdiff_t;

    array_view () = delete;

    template<typename ... ARGS>
    array_view (std::vector<DATA, ARGS...>& vec) noexcept
        : data_ (vec.data ()), size_ (vec.size ()) {}
    array_view (DATA* data, size_type size) : data_ (data), size_ (size) {}

    template<typename ... ARGS>
    array_view (std::basic_string<DATA, ARGS...>& vec) noexcept
    : data_ (const_cast<DATA*>(vec.data ())), size_ (vec.size ()) {}

    template<std::size_t size>
    array_view (DATA(&data) [size]) noexcept
    : data_ (data), size_ (size) {}

    template<std::size_t size>
    array_view (std::array<DATA, size>& array) noexcept
    :data_ (array.data ()), size_ (array.size ()) {}


    array_view operator= (const array_view<DATA>& that)
    {
        data_ = that.data_;
        size_ = that.size_;
        return *this;
    }

    template<typename POD, typename SFINAE = typename std::enable_if<std::is_pod<POD>::value, void>::type>
    array_view (POD& pod) noexcept
        :data_ (reinterpret_cast<DATA*>(&pod)), size_ (sizeof pod / sizeof (DATA)) { static_assert (sizeof pod % sizeof (DATA) == 0, "sizeof DATA is not 1");}

    array_view (const array_view&) = default;

    constexpr iterator begin () noexcept { return data_;}
    constexpr iterator end () noexcept { return data_ + size_;}
    constexpr reverse_iterator rbegin () noexcept { return std::reverse_iterator<iterator> (end ());}
    constexpr reverse_iterator rend () noexcept { return std::reverse_iterator<iterator> (begin ());}
    constexpr size_type size () const noexcept { return size_; }
    constexpr size_type length () const noexcept { return size_; }
    constexpr bool empty () const noexcept { return size_ == 0; }
    constexpr DATA& operator [] (size_type size) noexcept { return at (size); }
    constexpr const DATA& operator [] (size_type size) const noexcept { return at (size); }

    constexpr auto sub_view (unsigned pos, unsigned len) { if (pos + len > size_) throw std::out_of_range ("subview out of range"); return array_view<DATA> {data_ + pos, len};}
    constexpr auto sub_view (unsigned pos) { if (pos > size_) throw std::out_of_range ("subview out of range"); return array_view<DATA> {data_ + pos, size_ - pos};}

    constexpr DATA& front () { return *data_;}
    constexpr const DATA& front ()const { return *data_;}

    constexpr DATA& back () { return *(data_ + size_ - 1);}
    constexpr const DATA& back ()const { return *(data_ + size_ - 1);}

    constexpr DATA* data () { return data_;}
    constexpr const DATA* data ()const { return data_;}

    constexpr const DATA& at (size_type pos) const
    {
        if (pos < size_)
        {
            return data_[pos];
        }
        throw std::out_of_range ("at (pos) out of range");
    }

    constexpr DATA& at (size_type size)
    {
        return const_cast<DATA&>(const_cast<const array_view*> (this)->at (size));
    }


private:
    DATA* data_;
    uint32_t size_;
};
