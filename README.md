How to use

To control the number of messages sent and the size of each message, each "_bench" executable (which stars the server and client) takes two optional command-line arguments:

* `-c <count>`: How many messages to send between the server and client. Defaults to 1000
* `-s <size>`: The size of individual messages. Defaults to 4096.

E.g. for FIFO

```shell
cd build/fifo
./fifo_bench -c 10000 -s 90000
```