cmake_minimum_required(VERSION 3.31.6)
project(BashSpark)

# Project settings
set(CMAKE_CXX_STANDARD 20)
set(PROJECT_DIR ${CMAKE_CURRENT_LIST_DIR})

# Include directories (needed by IDE)

# External components
include(${PROJECT_DIR}/cmake/cmp.cmake)
include(${PROJECT_DIR}/cmake/doxygen.cmake)
include(${PROJECT_DIR}/cmake/nlohmann.cmake)
include(${PROJECT_DIR}/cmake/boost.cmake)

# Compilarion flags

# File management
# The IDE requires header files

set(HEADERS
        include/BashSpark/BashSpark.h
        include/BashSpark/command.h
        include/BashSpark/shell.h
        include/BashSpark/shell/shell_arg.h
        include/BashSpark/shell/shell_env.h
        include/BashSpark/shell/shell_parser_exception.h
        include/BashSpark/shell/shell_keyword.h
        include/BashSpark/shell/shell_node.h
        include/BashSpark/shell/shell_node_visitor.h
        include/BashSpark/shell/shell_node_visitor_json.h
        include/BashSpark/shell/shell_parser.h
        include/BashSpark/shell/shell_session.h
        include/BashSpark/shell/shell_status.h
        include/BashSpark/shell/shell_var.h
        include/BashSpark/shell/shell_vtable.h
        include/BashSpark/tools/fakestream.h
        include/BashSpark/tools/hash.h
        include/BashSpark/tools/nullstream.h
        include/BashSpark/tools/shell_hash.h
        include/BashSpark/tools/utf.h
        include/BashSpark/command/command_env.h
        include/BashSpark/command/command_fcall.h
        include/BashSpark/command/command_math.h
        include/BashSpark/command/command_seq.h
        include/BashSpark/command/command_test.h
        include/BashSpark/command/command_var.h
        include/BashSpark/tools/shell_def.h
)

set(SOURCES
        src/BashSpark/command.cpp
        src/BashSpark/shell.cpp
        src/BashSpark/shell/shell_parser_exception.cpp
        src/BashSpark/shell/shell_node.cpp
        src/BashSpark/shell/shell_node_evaluate.cpp
        src/BashSpark/shell/shell_node_expand.cpp
        src/BashSpark/shell/shell_node_visitor_json.cpp
        src/BashSpark/shell/shell_parser.cpp
        src/BashSpark/shell/shell_tokenizer.cpp
        src/BashSpark/shell/shell_tools.h
        src/BashSpark/shell/token_holder.h
        src/BashSpark/command/command_env.cpp
        src/BashSpark/command/command_fcall.cpp
        src/BashSpark/command/command_math.cpp
        src/BashSpark/command/command_seq.cpp
        src/BashSpark/command/command_test.cpp
        src/BashSpark/command/command_var.cpp
)

set(TEST_HEADERS
        test/include/BashSpark/test/test_shell.h
)

set(TEST_SOURCES
        test/src/BashSpark/test/test_shell.cpp
        src/BashSpark/test.cpp
)

# Libraries

# Static library (abashspark)
add_library(abashspark STATIC ${HEADERS} ${SOURCES})
set_target_properties(abashspark PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_DIR}/build)
add_cmp_flags(abashspark)

# Dynamic library (sbashspark)
add_library(sbashspark SHARED ${HEADERS} ${SOURCES})
set_target_properties(sbashspark PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_DIR}/build)
add_cmp_flags(sbashspark)

# Static library with position-independent code (fbashspark)
add_library(fbashspark STATIC ${HEADERS} ${SOURCES})
set_target_properties(fbashspark PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(fbashspark PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_DIR}/build)
add_cmp_flags(fbashspark)

# Dynamic library for testing (tbashspark)
add_executable(tbashspark ${TEST_HEADERS} ${TEST_SOURCES})
target_include_directories(tbashspark PRIVATE ${PROJECT_DIR}/test/include)
add_cmp_flags(tbashspark)

# Link libraries
target_link_libraries(tbashspark PRIVATE abashspark)

# Meta target
add_library(BashSpark::ABashSpark ALIAS abashspark)
add_library(BashSpark::FBashSpark ALIAS fbashspark)
add_library(BashSpark::SBashSpark ALIAS sbashspark)
add_library(BashSpark::BashSpark ALIAS sbashspark)

# Documentation target
add_custom_target(doc
        COMMAND ${DOXYGEN} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
)

# Headers target
set(HEADER_TAR "${PROJECT_DIR}/build/BashSpark.tar.gz")
add_custom_command(
        OUTPUT ${HEADER_TAR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_DIR}/build
        WORKING_DIRECTORY ${PROJECT_DIR}/include
        COMMAND ${CMAKE_COMMAND} -E tar -czvf ${HEADER_TAR}.gz .
        DEPENDS ${HEADERS}
        COMMENT "Gathering header files"
)
add_custom_target(headers
        DEPENDS ${HEADER_TAR}
        COMMENT "Gathering header files"
)


add_custom_target(MyOneTimeTarget DEPENDS output_file.txt)

# Debian package target
set(DEBIAN_DIR "${PROJECT_DIR}/resources/bashspark-1.0/debian")
set(DEBIAN_LIB "${DEBIAN_DIR}/usr/lib/BashSpark")
set(DEBIAN_INC "${DEBIAN_DIR}/usr/include")
add_custom_command(
        OUTPUT ${PROJECT_DIR}/build/bashspark_1.0_amd64.deb
        COMMAND mkdir -p "${DEBIAN_LIB}"
        COMMAND mkdir -p "${DEBIAN_INC}"
        COMMAND ln -f ${PROJECT_DIR}/build/libabashspark.a ${DEBIAN_LIB}/libabashspark.a
        COMMAND ln -f ${PROJECT_DIR}/build/libfbashspark.a ${DEBIAN_LIB}/libfbashspark.a
        COMMAND ln -f ${PROJECT_DIR}/build/libsbashspark.so ${DEBIAN_LIB}/libsbashspark.so
        COMMAND tar -xzf ${HEADER_TAR} -C ${DEBIAN_INC} --strip-components=1 "./BashSpark"
        COMMAND dpkg-deb --build ${DEBIAN_DIR} ${PROJECT_DIR}/build/bashspark_1.0_amd64.deb
        WORKING_DIRECTORY ${PROJECT_DIR}/resources/bashspark-1.0
        COMMENT "Building Debian package for BashSpark"
)
add_custom_target(deb DEPENDS ${PROJECT_DIR}/build/bashspark_1.0_amd64.deb)
add_dependencies(deb abashspark fbashspark sbashspark headers)

# Installation setup
install(DIRECTORY ${PROJECT_DIR}/include/ DESTINATION include)
install(TARGETS abashspark fbashspark sbashspark nlohmann_json
        EXPORT BashSparkTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        INCLUDES DESTINATION include
)
install(EXPORT BashSparkTargets
        FILE BashSparkTargets.cmake
        NAMESPACE BashSpark::
        DESTINATION lib/cmake/BashSpark
)
