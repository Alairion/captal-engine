dummy::dummy(VkDevice device)
:m_device{device}
{
    VkDummyCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DUMMY_CREATE_INFO;

    if(vkCreateDummy(m_device, &create_info, nullptr, &m_dummy) != VK_SUCCESS)
        throw std::runtime_error{"Failed to create dummy."};
}

dummy::~dummy()
{
    if(m_dummy)
        vkDestroyDummy(m_device, m_dummy, nullptr);
}

dummy::dummy(dummy&& other) noexcept
:m_device{other.m_device}
,m_dummy{std::exchange(other.m_dummy, nullptr)}
{

}

dummy& dummy::operator=(dummy&& other) noexcept
{
    m_device = other.m_device;
    m_dummy = std::exchange(other.m_dummy, nullptr);
    
    return *this;
}
