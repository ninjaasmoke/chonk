cmake_minimum_required(VERSION 3.10)
project(FileSplitterCLI)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenSSL REQUIRED)

add_executable(file_splitter main.cpp)

target_include_directories(file_splitter PRIVATE ${OPENSSL_INCLUDE_DIR})
target_link_libraries(file_splitter PRIVATE ${OPENSSL_LIBRARIES})
