add_executable(mmap_client mmap_client.c)
add_executable(mmap_server mmap_server.c)
add_executable(mmap_bench mmap_bench.c)

target_link_libraries(mmap_client common)
target_link_libraries(mmap_server common)
target_link_libraries(mmap_bench common)