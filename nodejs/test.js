
const path = require('path')
const smipc = require('./build/Release/smipc.node')
smipc.loadSmipc(path.join(__dirname, 'lib/libsmipc.dll'))
delete smipc['loadSmipc']

const testCid = "test-chan"
const dataSz = 1024
args = process.argv.slice(2)
if (args[0] == '-r') {
    smipc.init(smipc.LOG_ALL)
    smipc.openChannel(testCid, smipc.CHAN_R, 12)
    // generate test data
    let buf = new Uint8Array(dataSz)
    // send data
    smipc.printChannelStatus(testCid)
    let n = smipc.readChannel(testCid, buf, dataSz, true)
    console.log('read ' + n)
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
} else if (args[0] == '-w') {
    smipc.init(smipc.LOG_ALL)
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
