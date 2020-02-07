class dummy
{
public:
    constexpr dummy() = default;
    dummy(VkDevice device);
    ~dummy();

    dummy(const dummy&) = delete;
    dummy& operator=(const dummy&) = delete;

    dummy(dummy&& other) noexcept;
    dummy& operator=(dummy&& other) noexcept;

    explicit dummy(VkDevice device, VkDummy dummy) noexcept
    :m_device{device}
    ,m_dummy{dummy}
    {
        
    }
    
    operator VkDummy() const noexcept
    {
        return m_dummy;
    }

private:
    VkDevice m_device{};
    VkDummy m_dummy{};
};

//

#ifdef TPH_PLATFORM_ANDROID

#endif
#ifdef TPH_PLATFORM_IOS

#endif
#ifdef TPH_PLATFORM_WIN32

#endif
#ifdef TPH_PLATFORM_MACOS

#endif
#ifdef TPH_PLATFORM_XLIB

#endif
#ifdef TPH_PLATFORM_XCB

#endif
#ifdef TPH_PLATFORM_WAYLAND

#endif

    using value_type = pixel;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = pixel*;
    using const_pointer = const pixel*;
    using reference = pixel&;
    using const_reference = const pixel&;
    using iterator = pixel*;
    using const_iterator = const pixel*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    pointer data() noexcept;
    const_pointer data() const noexcept;
    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    const_iterator cbegin() const noexcept;
    iterator end() noexcept;
    const_iterator end() const noexcept;
    const_iterator cend() const noexcept;
    reverse_iterator rbegin() noexcept;
    const_reverse_iterator rbegin() const noexcept;
    const_reverse_iterator crbegin() const noexcept;
    reverse_iterator rend() noexcept;
    const_reverse_iterator rend() const noexcept;
    const_reverse_iterator crend() const noexcept;
    size_type size() const noexcept;
    size_type max_size() const noexcept;