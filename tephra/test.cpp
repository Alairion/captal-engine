
    
    std::minstd_rand engine{static_cast<std::uint32_t>(std::chrono::system_clock::now().time_since_epoch().count())};

    const auto tp1{std::chrono::system_clock::now()};

    std::vector<tph::vulkan::memory_heap_chunk> chunks{};

    chunks.reserve((1 << 17));
    for(std::size_t i{}; i < (1 << 17); ++i)
    {
        VkMemoryRequirements requirements{};
        tph::vulkan::memory_ressource_type ressource_type{};
        VkMemoryPropertyFlags required{};
        VkMemoryPropertyFlags optimal{};

        if(i % 5 == 0 || i % 5 == 3)
        {
            requirements.size = 128/* << (i % 16)*/;
            requirements.alignment = 256;
            requirements.memoryTypeBits = 0xFFFFFFFF;
            ressource_type = tph::vulkan::memory_ressource_type::linear;
            required = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        }
        else if(i % 5 == 1)
        {
            requirements.size = 1024 /*<< (i % 16)*/;
            requirements.alignment = 128;
            requirements.memoryTypeBits = 0xFFFFFFFF;
            ressource_type = tph::vulkan::memory_ressource_type::linear;
            required = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            optimal = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        }
        else
        {
            requirements.size = 512 /*<< (i % 16)*/;
            requirements.alignment = 1024;
            requirements.memoryTypeBits = 0xFFFFFFFF;
            ressource_type = tph::vulkan::memory_ressource_type::non_linear;
            required = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        }

        chunks.push_back(renderer.allocator().allocate(requirements, ressource_type, required, optimal));

        if(i % 1024 == 1023)
        {
            for(std::size_t i{}; i < 512; ++i)
            {
                std::uniform_int_distribution<std::size_t> dist{0, std::size(chunks) - 1};
                chunks.erase(std::begin(chunks) + dist(engine));
            }
        }
    }

    std::cout << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(std::chrono::system_clock::now() - tp1).count() << "ms" << std::endl;

    chunks.clear();
    renderer.free_memory();

    std::cout << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(std::chrono::system_clock::now() - tp1).count() << "ms" << std::endl;

