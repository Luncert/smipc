# smipc

This package is used to transport big data between processes, bases on shared memory.

> For now, only windows supported.

## API

* ```init(isTraceMode: bool): void```: initialize library, make sure call it firstly
  * ```isTraceMode```: print some debug info if set true.
* ```openChannel(cid: string, mode: number, chanSz: number): bool```
  * ```mode```: CHAN_R=open channel as reader, CHAN_W=open channel as writer
* ```readChannel(cid: string, buf: Uint8Array, sz: number, blocking: bool): number```
  * ```blocking```: if set true, block operation until received the specified amount of data or opposite end closed
* ```writeChannel(cid: string, data: Uint8Array, sz: number): bool```
* ```printChannelStatus(cid: string): bool```
* ```closeChannel(cid: string): bool```
* ```deinit(): void```: clean library

## Demo

```js
const smipc = require('smipc')

const testCid = "test-chan"
const dataSz = 1024

args = process.argv.slice(2)
if (args[0] == '-r') {
    // init library
    smipc.init(true)
    smipc.openChannel(testCid, smipc.CHAN_R, 12)
    // generate test data
    let buf = new Uint8Array(dataSz)
    // send data
    smipc.printChannelStatus(testCid)
    let n = smipc.readChannel(testCid, buf, dataSz, true)
    console.log('read ' + n)
    smipc.printChannelStatus(testCid)
    // close channel
    smipc.closeChannel(testCid)
    // clean library
    smipc.deinit()
    // print test data
    let s = ''
    for (let i = 0; i < dataSz; i++) {
        s += buf[i] + ' '
    }
    console.log(s)
} else if (args[0] == '-w') {
    smipc.init(true)
    smipc.openChannel(testCid, smipc.CHAN_W, 12)
    // generate test data
    let buf = new Uint8Array(dataSz)
    for (let i = 0; i < dataSz; i++) {
        buf[i] = Math.round(Math.random() * 255)
    }
    // send data
    smipc.printChannelStatus(testCid)
    smipc.writeChannel(testCid, buf, dataSz)
    smipc.printChannelStatus(testCid)
    // clean
    smipc.closeChannel(testCid)
    smipc.deinit()
    // print test data
    let s = ''
    for (let i = 0; i < dataSz; i++) {
        s += buf[i] + ' '
    }
    console.log(s)
} else {
    console.error('Invalid option.')
}
```

