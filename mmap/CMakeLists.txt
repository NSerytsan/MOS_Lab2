add_executable(mmap_client mmap_client.c)
add_executable(mmap_server mmap_server.c)

target_link_libraries(mmap_client common)
target_link_libraries(mmap_server common)