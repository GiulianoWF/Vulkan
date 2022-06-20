
add_executable(vulkanApp main.cc)

# target_include_directories(activity_manager PUBLIC include)

# find_package(OpenSSL)
find_package(GLEW)
find_package(glfw3)
find_package(VulkanHeaders)
find_package(glm)
find_package(stb)
find_package(tinyobjloader)
# find_package(vulkan)
find_package(Boost)
find_package(volk)

target_link_libraries(vulkanApp GLEW::GLEW glfw Vulkan::Vulkan vulkan glm::glm stb::stb tinyobjloader Boost::boost)

add_executable(vertexProcessing minimalVulkan.cc)
target_link_libraries(vertexProcessing Boost::boost GLEW::GLEW glfw Vulkan::Vulkan vulkan glm::glm)
