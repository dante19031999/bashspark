
include(FetchContent)

# Boost numerics
FetchContent_Declare(
        safe_numerics
        GIT_REPOSITORY https://github.com/robertramey/safe_numerics.git
        GIT_TAG        master   # or a specific tag like v1.0.0
)
FetchContent_MakeAvailable(safe_numerics)
set(BOOST_NUMERIC_HEADERS "${safe_numerics_SOURCE_DIR}/include")
message(STATUS "boost ${BOOST_NUMERIC_HEADERS}")
