#ifndef CAPTAL_FOUNDATION_ENDIAN_HPP_INCLUDED
#define CAPTAL_FOUNDATION_ENDIAN_HPP_INCLUDED

#include <cstdint>
#include <new>
#include <memory>
#include <array>
#include <stdexcept>
#include <cassert>
#include <cstddef>
#include <string>
#include <cstring>
#include <vector>

namespace cpt
{

inline namespace foundation
{

template<std::size_t StackSize>
class stack_memory_pool
{
public:
    static constexpr std::size_t block_align{alignof(std::max_align_t)};
    static constexpr std::size_t block_size{block_align};

private:
    static constexpr std::size_t align(std::size_t size) noexcept
    {
        return (size + block_align - 1) & ~(block_align - 1);
    }

public:
    static constexpr std::size_t stack_size{align(StackSize)};

private:
    static constexpr std::size_t block_count{stack_size / block_size};
    static constexpr std::size_t used_mask{0x01};
    using block_type = std::array<std::uint8_t, block_size>;

private:
    static constexpr std::size_t& as_header(block_type& block) noexcept
    {
        return *std::launder(reinterpret_cast<std::size_t*>(&block));
    }

    static constexpr bool is_empty(block_type block) noexcept
    {
        return as_header(block) == 0;
    }

    static constexpr bool is_used(block_type block) noexcept
    {
        return (as_header(block) & used_mask) != 0;
    }

    static constexpr std::size_t get_size(block_type block) noexcept
    {
        return as_header(block) & ~used_mask;
    }

public:
    constexpr stack_memory_pool() noexcept
    {
        as_header(m_memory[0]) = std::numeric_limits<std::size_t>::max(); //We do lazy initialization
    }

     ~stack_memory_pool() = default;
    stack_memory_pool(const stack_memory_pool&) noexcept = delete;
    stack_memory_pool& operator=(const stack_memory_pool&) noexcept = delete;
    stack_memory_pool(stack_memory_pool&&) noexcept = delete;
    stack_memory_pool& operator=(stack_memory_pool&&) noexcept = delete;

    constexpr void* allocate(std::size_t size) noexcept
    {
        assert(size > 0);

        size = align(size);
        if(size > stack_size - block_size)
        {
            return nullptr;
        }

        if(as_header(m_memory[0]) == std::numeric_limits<std::size_t>::max())
        {
            std::memset(std::data(m_memory), 0, std::size(m_memory) * block_size);
        }

        auto* const block{find_block(size)};
        auto* const user_block_begin{block + 1};
        auto* const user_block_end{user_block_begin + (size / block_size)};

        if(user_block_end <= std::data(m_memory) + std::size(m_memory))
        {
            write_size(block, size);
            return user_block_begin;
        }

        return nullptr;
    }

    constexpr void deallocate(void* user_pointer) noexcept
    {
        auto* const pointer{reinterpret_cast<block_type*>(user_pointer) - 1};

        assert(own(pointer));
        assert(is_used(*pointer));

        as_header(*pointer) &= ~used_mask;
    }

    constexpr bool own(void* pointer) const noexcept
    {
        return std::data(m_memory) <= pointer && pointer < std::data(m_memory) + std::size(m_memory);
    }

private:
    constexpr block_type* find_block(std::size_t size) noexcept
    {
        block_type* begin{std::data(m_memory)};
        block_type* const end{std::data(m_memory) + std::size(m_memory)};

        while(as_header(*begin) && begin != end) //Header with value 0 indicates end of used space
        {
            const block_type header{*begin};
            const std::size_t header_size{get_size(header)};

            if(!is_used(header) && header_size >= size)
            {
                return begin;
            }

            begin += 1 + header_size / block_size;
        }

        return begin;
    }

    constexpr void write_size(block_type* block, std::size_t size) noexcept
    {
        const std::size_t old_size{get_size(*block)};
        if(old_size > 0 && old_size != size) //check if block has been used at least once
        {
            block_type* const next{block + (old_size / block_size)};
            if(next < std::data(m_memory) + std::size(m_memory) && get_size(*next)) //check if block is not the last
            {
                block_type* const inter{block + (size / block_size)};
                const std::size_t inter_size{old_size - (size + block_size)};

                if(inter_size > 0) //prevent from wasting space
                {
                    as_header(*inter) = inter_size;
                }
                else
                {
                    as_header(*block) = old_size | used_mask; //keep old size and mark it as used
                }
            }
        }

        as_header(*block) = size | used_mask; //change it's size and mark it as used
    }

private:
    alignas(block_align) std::array<block_type, block_count> m_memory;
};

template<typename T, std::size_t StackSize, bool NewFallback = true>
class stack_allocator
{
public:
    static constexpr std::size_t type_size{sizeof(T)};
    static constexpr bool new_fallback{NewFallback};
    static constexpr std::size_t stack_size{StackSize};

public:
    using value_type = T;
    using memory_pool_type = stack_memory_pool<stack_size>;
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
    using is_always_equal = std::false_type;

public:
    template<typename U>
    struct rebind
    {
        using other = stack_allocator<U, stack_size, new_fallback>;
    };

public:
    constexpr stack_allocator() noexcept = default;

    constexpr stack_allocator(memory_pool_type& pool) noexcept
    :m_pool{&pool}
    {

    }

    template<typename U>
    constexpr stack_allocator(const stack_allocator<U, stack_size, new_fallback>& other) noexcept
    :m_pool{other.m_pool}
    {

    }

    constexpr ~stack_allocator() = default;
    constexpr stack_allocator(const stack_allocator&) noexcept = default;
    constexpr stack_allocator& operator=(const stack_allocator&) noexcept = default;
    constexpr stack_allocator(stack_allocator&&) noexcept = default;
    constexpr stack_allocator& operator=(stack_allocator&&) noexcept = default;

    constexpr T* allocate(std::size_t count)
    {
        const auto ptr{m_pool->allocate(type_size * count)};

        if(!ptr)
        {
            if constexpr(new_fallback)
            {
                return reinterpret_cast<T*>(::operator new(type_size * count));
            }
            else
            {
                throw std::bad_alloc{};
            }
        }

        return reinterpret_cast<T*>(ptr);
    }

    constexpr void deallocate(T* ptr, std::size_t count)
    {
        if constexpr(new_fallback)
        {
            if(m_pool->own(ptr))
            {
                m_pool->deallocate(ptr);
            }
            else
            {
                ::operator delete(ptr, type_size * count);
            }
        }
        else
        {
            m_pool->deallocate(ptr);
        }
    }

    constexpr memory_pool_type& memory_pool() const noexcept
    {
        return *m_pool;
    }

private:
    memory_pool_type* m_pool{};
};

template<typename T, typename U, std::size_t StackSize, bool NewFallback>
constexpr bool operator==(const stack_allocator<T, StackSize, NewFallback>& right, const stack_allocator<U, StackSize, NewFallback>& left) noexcept
{
    return right.memory_pool() == left.memory_pool();
}

template<typename T, typename U, std::size_t StackSize, bool NewFallback>
constexpr bool operator!=(const stack_allocator<T, StackSize, NewFallback>& right, const stack_allocator<U, StackSize, NewFallback>& left) noexcept
{
    return !(left == right);
}

template<typename T, std::size_t StackSize>
using stack_vector = std::vector<T, stack_allocator<T, StackSize>>;

template<typename T, std::size_t StackSize>
constexpr stack_vector<T, StackSize> make_stack_vector(stack_memory_pool<StackSize>& pool) noexcept
{
    return stack_vector<T, StackSize>{pool};
}

template<std::size_t StackSize, typename CharT, typename Traits = std::char_traits<CharT>>
using stack_basic_string = std::basic_string<CharT, Traits, stack_allocator<CharT, StackSize>>;

template<std::size_t StackSize> using stack_string    = stack_basic_string<StackSize, char>;
template<std::size_t StackSize> using stack_wstring   = stack_basic_string<StackSize, wchar_t>;
template<std::size_t StackSize> using stack_u8string  = stack_basic_string<StackSize, char8_t>;
template<std::size_t StackSize> using stack_u16string = stack_basic_string<StackSize, char16_t>;
template<std::size_t StackSize> using stack_u32string = stack_basic_string<StackSize, char32_t>;

template<typename T, std::size_t StackSize>
constexpr stack_string<StackSize> make_stack_string(stack_memory_pool<StackSize>& pool) noexcept
{
    return stack_string<StackSize>{pool};
}

template<typename T, std::size_t StackSize>
constexpr stack_wstring<StackSize> make_stack_wstring(stack_memory_pool<StackSize>& pool) noexcept
{
    return stack_wstring<StackSize>{pool};
}

template<typename T, std::size_t StackSize>
constexpr stack_u8string<StackSize> make_stack_u8string(stack_memory_pool<StackSize>& pool) noexcept
{
    return stack_u8string<StackSize>{pool};
}

template<typename T, std::size_t StackSize>
constexpr stack_u16string<StackSize> make_stack_u16string(stack_memory_pool<StackSize>& pool) noexcept
{
    return stack_u16string<StackSize>{pool};
}

template<typename T, std::size_t StackSize>
constexpr stack_u32string<StackSize> make_stack_u32string(stack_memory_pool<StackSize>& pool) noexcept
{
    return stack_u32string<StackSize>{pool};
}

}

}

#endif
