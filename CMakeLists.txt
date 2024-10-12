cmake_minimum_required(VERSION 3.5)
project(MOS_Lab2)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_compile_options(-std=c17 -O3 -Wall -Werror -D_XOPEN_SOURCE -D_POSIX_C_SOURCE=200809L)
link_libraries(m pthread)

add_subdirectory(common)
add_subdirectory(mmap)
add_subdirectory(shm)
add_subdirectory(fifo)
add_subdirectory(unix_domain)
add_subdirectory(tcp)
