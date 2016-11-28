#ifndef ASSERT_EXCEPTION_H
#define ASSERT_EXCEPTION_H
#include <exception>
#include <array>

#define assure(exp) \
    if (!(exp)) throw exp_assert ("%s\nfunction:%s\nfile:%s\nline:%d\n", #exp, __PRETTY_FUNCTION__, __FILE__, __LINE__);

class exp_assert : public std::exception
{
public:
    template<typename ... ARGS>
    exp_assert (ARGS&& ... args)
    {
        ::snprintf(buffer_.data(), buffer_.size() - 1, std::forward<ARGS> (args)...);
    }

    const char* what ()const noexcept override
    {
        return buffer_.data();
    }

private:
    std::array<char, 256> buffer_;
};



#endif // ASSERT_EXCEPTION_H
