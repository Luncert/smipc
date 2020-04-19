# smipc for python

This module is a fast ipc library based on shared memory.

> For now, only windows is supported.

## API

* ```init_library(logLevel): void```: initializa library and set log level, supported log level: ```LOG_DISABLE```, ```LOG_BASIC```, ```LOG_ALL```.
* ```open_channel(cid, mode, chanSz): Channel```: create a instance of **Channel**, cid must be **bytes**, like: ```b'test-chan'```.
  * **Channel** implements magic methods for context management ```__enter__``` and ```__exit__```: it opens a channel with specified cid, mode (```CHAN_R```, ```CHAN_W```) and chanSz (channel size) in ```__enter__```, and closes the channel automatically in ```__exit__```.
  * **If there is a channel having the same cid**, library will use it directly and the chanSz parameter will be ignored, in another word, chanSz only makes sense when you do create a new channel.
  * **A channel only allows one writer and one reader.** And both of the read and write operation are **not thread-safe**, wrap with a lock if you want to do concurrently read or write.
* ```Channel.print_status(): void```: print current status of the channel in such format:
  ```
  Channel(cid): mode=[R|W] hSharedMem=handleOfSharedMem listener=ifEnableListener syncBuf={RO=ifReaderOpen WO=ifWriterOpen RC=ifReaderClosed WC=ifWriterClosed hREvt=handleOfReadEvent hWEvt=handleOfWriteEvent bufSz=channelSize rc=readCursor wc=WriterCursor state=[empty|full|normal]}
  ```
* ```Channel.write(data): void```: data must be **bytes**, example: ```bytes([32, 22, 65])```.
  * if the channel is full of unconsumed data, invoking write will be blocked until reader read some data.
* ```Channel.read(buf, n, blocking=False): void```:
  * use python builtin function ```create_string_buffer(size)``` to create **buf**.
  * **n** indicates the number of bytes to read from channel.
  * if **blocking** is set to true, this call won't return until channel read **n** bytes.
* ```clean_library(): void```: indicate library to release resources, notice if the opposite end (writer or reader) is still alive, the channel won't be closed.

## Example

Blocking transport:
* [sender](/py/example/sender.py)
* [receiver](/py/example/receiver.py)

Async trasnport:
* [sender](/py/example/sender_async.py)
* [receiver](/py/example/receiver_async.py)