add_library(CustomVulkan SHARED IMPORTED)
# add_dependencies(CustomVulkan PUBLIC external_mujoco)
target_include_directories(CustomVulkan INTERFACE /home/giuliano/Downloads/1.3.216.0/x86_64)
set_target_properties(CustomVulkan PROPERTIES IMPORTED_LOCATION "/home/giuliano/Downloads/1.3.216.0/x86_64/lib/libvulkan.so")
# target_link_libraries(CustomVulkan INTERFACE ${GLFW3_LIB} ${GLEW_LIB} ${OPENGL_gl_LIBRARY})