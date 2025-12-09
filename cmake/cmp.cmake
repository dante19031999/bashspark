
# Add compile options to target
message(STATUS "aaaaaaaaaa")

function(add_cmp_flags target)
    # Includes
    target_include_directories("${target}"
            PUBLIC
            $<BUILD_INTERFACE:${PROJECT_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )
    target_include_directories("${target}" PRIVATE "${BOOST_NUMERIC_HEADERS}")
    # Includes property
    set_target_properties("${target}" PROPERTIES
            BASHSPARK_HEADER_DIR "$<BUILD_INTERFACE:${PROJECT_DIR}/include>"
    )
    # Link json
    target_link_libraries("${target}" PUBLIC nlohmann_json::nlohmann_json)
    # Compiler flags
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "debug")
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options("${target}" PRIVATE -O0 -g)
        elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            target_compile_options("${target}" PRIVATE /Zi /Od)
        elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            target_compile_options("${target}" PRIVATE -O0 -g)
        endif ()
    else ()
        message(STATUS "here")
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options("${target}" PRIVATE -O2)
            get_target_property(linkflags ${target} LINK_FLAGS)
            if(NOT linkflags)
                set(linkflags "")
            endif()
            set(linkflags "${linkflags} -flto")
            set_target_properties("${target}" PROPERTIES LINK_FLAGS ${linkflags})
        elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            target_compile_options("${target}" PRIVATE /O2)
            get_target_property(linkflags ${target} LINK_FLAGS)
            if(NOT linkflags)
                set(linkflags "")
            endif()
            set(linkflags "${linkflags} /LTCG")
            set_target_properties("${target}" PROPERTIES LINK_FLAGS ${linkflags})
        elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            target_compile_options("${target}" PRIVATE -O2)
            get_target_property(linkflags ${target} LINK_FLAGS)
            if(NOT linkflags)
                set(linkflags "")
            endif()
            set(linkflags "${linkflags} -flto")
            set_target_properties("${target}" PROPERTIES LINK_FLAGS ${linkflags})
        endif ()
    endif ()
endfunction()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DBS_DEBUG)
endif()
