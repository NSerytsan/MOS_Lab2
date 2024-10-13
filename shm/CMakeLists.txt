add_executable(shm_client shm_client.c)
add_executable(shm_server shm_server.c)
add_executable(shm_bench shm_bench.c)

target_link_libraries(shm_client common)
target_link_libraries(shm_server common)
target_link_libraries(shm_bench common)