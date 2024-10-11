cmake_minimum_required(VERSION 3.5)
project(MOS_Lab2)

add_compile_options(-std=c17 -O3 -Wall -Werror)

add_subdirectory(mmap)
add_subdirectory(shm)
add_subdirectory(fifo)
add_subdirectory(unix_domain)
add_subdirectory(tcp)
