cmake_minimum_required(VERSION 3.5)
project(MOS_Lab2)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_compile_options(-std=gnu17 -O3 -Wall -Werror -pedantic -D_GNU_SOURCE)
link_libraries(m pthread)

add_subdirectory(common)
add_subdirectory(mmap)
add_subdirectory(fifo)
add_subdirectory(pipe)
add_subdirectory(shm)
add_subdirectory(unix_domain)
add_subdirectory(tcp)
