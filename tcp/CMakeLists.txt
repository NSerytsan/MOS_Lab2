add_executable(tcp_client tcp_client.c)
add_executable(tcp_server tcp_server.c)
add_executable(tcp_bench tcp_bench.c)

target_link_libraries(tcp_client common)
target_link_libraries(tcp_server common)
target_link_libraries(tcp_bench common)