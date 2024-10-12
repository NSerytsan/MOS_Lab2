add_executable(fifo_client fifo_client.c)
add_executable(fifo_server fifo_server.c)

target_link_libraries(fifo_client common)
target_link_libraries(fifo_server common)