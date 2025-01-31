cmake_minimum_required(VERSION 3.10)
project(Chonk)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Include directories
include_directories(${OPENSSL_INCLUDE_DIR} include)

# Source files
set(SOURCES
    src/main.cpp
    src/banner.cpp
    src/encryption.cpp
    src/file_operations.cpp
    src/input_utils.cpp
)

# Add executable
add_executable(chonk ${SOURCES})

# Set the output directory for the executable to be one folder up
set_target_properties(chonk PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/..)

# Link OpenSSL libraries
target_link_libraries(chonk PRIVATE OpenSSL::Crypto OpenSSL::SSL)