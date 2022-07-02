

function(conan_message MESSAGE_OUTPUT)
    if(NOT CONAN_CMAKE_SILENT_OUTPUT)
        message(${ARGV${0}})
    endif()
endfunction()


macro(conan_find_apple_frameworks FRAMEWORKS_FOUND FRAMEWORKS FRAMEWORKS_DIRS)
    if(APPLE)
        foreach(_FRAMEWORK ${FRAMEWORKS})
            # https://cmake.org/pipermail/cmake-developers/2017-August/030199.html
            find_library(CONAN_FRAMEWORK_${_FRAMEWORK}_FOUND NAME ${_FRAMEWORK} PATHS ${FRAMEWORKS_DIRS} CMAKE_FIND_ROOT_PATH_BOTH)
            if(CONAN_FRAMEWORK_${_FRAMEWORK}_FOUND)
                list(APPEND ${FRAMEWORKS_FOUND} ${CONAN_FRAMEWORK_${_FRAMEWORK}_FOUND})
            else()
                message(FATAL_ERROR "Framework library ${_FRAMEWORK} not found in paths: ${FRAMEWORKS_DIRS}")
            endif()
        endforeach()
    endif()
endmacro()


function(conan_package_library_targets libraries package_libdir deps out_libraries out_libraries_target build_type package_name)
    unset(_CONAN_ACTUAL_TARGETS CACHE)
    unset(_CONAN_FOUND_SYSTEM_LIBS CACHE)
    foreach(_LIBRARY_NAME ${libraries})
        find_library(CONAN_FOUND_LIBRARY NAME ${_LIBRARY_NAME} PATHS ${package_libdir}
                     NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
        if(CONAN_FOUND_LIBRARY)
            conan_message(STATUS "Library ${_LIBRARY_NAME} found ${CONAN_FOUND_LIBRARY}")
            list(APPEND _out_libraries ${CONAN_FOUND_LIBRARY})
            if(NOT ${CMAKE_VERSION} VERSION_LESS "3.0")
                # Create a micro-target for each lib/a found
                string(REGEX REPLACE "[^A-Za-z0-9.+_-]" "_" _LIBRARY_NAME ${_LIBRARY_NAME})
                set(_LIB_NAME CONAN_LIB::${package_name}_${_LIBRARY_NAME}${build_type})
                if(NOT TARGET ${_LIB_NAME})
                    # Create a micro-target for each lib/a found
                    add_library(${_LIB_NAME} UNKNOWN IMPORTED)
                    set_target_properties(${_LIB_NAME} PROPERTIES IMPORTED_LOCATION ${CONAN_FOUND_LIBRARY})
                    set(_CONAN_ACTUAL_TARGETS ${_CONAN_ACTUAL_TARGETS} ${_LIB_NAME})
                else()
                    conan_message(STATUS "Skipping already existing target: ${_LIB_NAME}")
                endif()
                list(APPEND _out_libraries_target ${_LIB_NAME})
            endif()
            conan_message(STATUS "Found: ${CONAN_FOUND_LIBRARY}")
        else()
            conan_message(STATUS "Library ${_LIBRARY_NAME} not found in package, might be system one")
            list(APPEND _out_libraries_target ${_LIBRARY_NAME})
            list(APPEND _out_libraries ${_LIBRARY_NAME})
            set(_CONAN_FOUND_SYSTEM_LIBS "${_CONAN_FOUND_SYSTEM_LIBS};${_LIBRARY_NAME}")
        endif()
        unset(CONAN_FOUND_LIBRARY CACHE)
    endforeach()

    if(NOT ${CMAKE_VERSION} VERSION_LESS "3.0")
        # Add all dependencies to all targets
        string(REPLACE " " ";" deps_list "${deps}")
        foreach(_CONAN_ACTUAL_TARGET ${_CONAN_ACTUAL_TARGETS})
            set_property(TARGET ${_CONAN_ACTUAL_TARGET} PROPERTY INTERFACE_LINK_LIBRARIES "${_CONAN_FOUND_SYSTEM_LIBS};${deps_list}")
        endforeach()
    endif()

    set(${out_libraries} ${_out_libraries} PARENT_SCOPE)
    set(${out_libraries_target} ${_out_libraries_target} PARENT_SCOPE)
endfunction()


include(FindPackageHandleStandardArgs)

conan_message(STATUS "Conan: Using autogenerated Findglfw3.cmake")
# Global approach
set(glfw3_FOUND 1)
set(glfw3_VERSION "3.3.7")

find_package_handle_standard_args(glfw3 REQUIRED_VARS
                                  glfw3_VERSION VERSION_VAR glfw3_VERSION)
mark_as_advanced(glfw3_FOUND glfw3_VERSION)


set(glfw_INCLUDE_DIRS "/home/giuliano/.conan/data/glfw/3.3.7/_/_/package/495b95917c60701459b219bfbd75ba817bd2ec98/include")
set(glfw_INCLUDE_DIR "/home/giuliano/.conan/data/glfw/3.3.7/_/_/package/495b95917c60701459b219bfbd75ba817bd2ec98/include")
set(glfw_INCLUDES "/home/giuliano/.conan/data/glfw/3.3.7/_/_/package/495b95917c60701459b219bfbd75ba817bd2ec98/include")
set(glfw_RES_DIRS )
set(glfw_DEFINITIONS )
set(glfw_LINKER_FLAGS_LIST
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:>"
)
set(glfw_COMPILE_DEFINITIONS )
set(glfw_COMPILE_OPTIONS_LIST "" "")
set(glfw_COMPILE_OPTIONS_C "")
set(glfw_COMPILE_OPTIONS_CXX "")
set(glfw_LIBRARIES_TARGETS "") # Will be filled later, if CMake 3
set(glfw_LIBRARIES "") # Will be filled later
set(glfw_LIBS "") # Same as glfw_LIBRARIES
set(glfw_SYSTEM_LIBS m pthread dl rt)
set(glfw_FRAMEWORK_DIRS )
set(glfw_FRAMEWORKS )
set(glfw_FRAMEWORKS_FOUND "") # Will be filled later
set(glfw_BUILD_MODULES_PATHS "/home/giuliano/.conan/data/glfw/3.3.7/_/_/package/495b95917c60701459b219bfbd75ba817bd2ec98/lib/cmake/conan-official-glfw-targets.cmake")

conan_find_apple_frameworks(glfw_FRAMEWORKS_FOUND "${glfw_FRAMEWORKS}" "${glfw_FRAMEWORK_DIRS}")

mark_as_advanced(glfw_INCLUDE_DIRS
                 glfw_INCLUDE_DIR
                 glfw_INCLUDES
                 glfw_DEFINITIONS
                 glfw_LINKER_FLAGS_LIST
                 glfw_COMPILE_DEFINITIONS
                 glfw_COMPILE_OPTIONS_LIST
                 glfw_LIBRARIES
                 glfw_LIBS
                 glfw_LIBRARIES_TARGETS)

# Find the real .lib/.a and add them to glfw_LIBS and glfw_LIBRARY_LIST
set(glfw_LIBRARY_LIST glfw3)
set(glfw_LIB_DIRS "/home/giuliano/.conan/data/glfw/3.3.7/_/_/package/495b95917c60701459b219bfbd75ba817bd2ec98/lib")

# Gather all the libraries that should be linked to the targets (do not touch existing variables):
set(_glfw_DEPENDENCIES "${glfw_FRAMEWORKS_FOUND} ${glfw_SYSTEM_LIBS} opengl::opengl;xorg::xorg")

conan_package_library_targets("${glfw_LIBRARY_LIST}"  # libraries
                              "${glfw_LIB_DIRS}"      # package_libdir
                              "${_glfw_DEPENDENCIES}"  # deps
                              glfw_LIBRARIES            # out_libraries
                              glfw_LIBRARIES_TARGETS    # out_libraries_targets
                              ""                          # build_type
                              "glfw")                                      # package_name

set(glfw_LIBS ${glfw_LIBRARIES})

foreach(_FRAMEWORK ${glfw_FRAMEWORKS_FOUND})
    list(APPEND glfw_LIBRARIES_TARGETS ${_FRAMEWORK})
    list(APPEND glfw_LIBRARIES ${_FRAMEWORK})
endforeach()

foreach(_SYSTEM_LIB ${glfw_SYSTEM_LIBS})
    list(APPEND glfw_LIBRARIES_TARGETS ${_SYSTEM_LIB})
    list(APPEND glfw_LIBRARIES ${_SYSTEM_LIB})
endforeach()

# We need to add our requirements too
set(glfw_LIBRARIES_TARGETS "${glfw_LIBRARIES_TARGETS};opengl::opengl;xorg::xorg")
set(glfw_LIBRARIES "${glfw_LIBRARIES};opengl::opengl;xorg::xorg")

set(CMAKE_MODULE_PATH "/home/giuliano/.conan/data/glfw/3.3.7/_/_/package/495b95917c60701459b219bfbd75ba817bd2ec98/" ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH "/home/giuliano/.conan/data/glfw/3.3.7/_/_/package/495b95917c60701459b219bfbd75ba817bd2ec98/" ${CMAKE_PREFIX_PATH})

if(NOT ${CMAKE_VERSION} VERSION_LESS "3.0")
    # Target approach
    if(NOT TARGET glfw::glfw)
        add_library(glfw::glfw INTERFACE IMPORTED)
        if(glfw_INCLUDE_DIRS)
            set_target_properties(glfw::glfw PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                  "${glfw_INCLUDE_DIRS}")
        endif()
        set_property(TARGET glfw::glfw PROPERTY INTERFACE_LINK_LIBRARIES
                     "${glfw_LIBRARIES_TARGETS};${glfw_LINKER_FLAGS_LIST}")
        set_property(TARGET glfw::glfw PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     ${glfw_COMPILE_DEFINITIONS})
        set_property(TARGET glfw::glfw PROPERTY INTERFACE_COMPILE_OPTIONS
                     "${glfw_COMPILE_OPTIONS_LIST}")
        
        # Library dependencies
        include(CMakeFindDependencyMacro)

        if(NOT opengl_system_FOUND)
            find_dependency(opengl_system REQUIRED)
        else()
            message(STATUS "Dependency opengl_system already found")
        endif()


        if(NOT xorg_FOUND)
            find_dependency(xorg REQUIRED)
        else()
            message(STATUS "Dependency xorg already found")
        endif()

    endif()
endif()

foreach(_BUILD_MODULE_PATH ${glfw_BUILD_MODULES_PATHS})
    include(${_BUILD_MODULE_PATH})
endforeach()
