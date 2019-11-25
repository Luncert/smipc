const addon = require('./build/Release/smipc.node')

const testCid = "test-chan"
const dataSz = 20
args = process.argv.slice(2)
if (args[0] == '-r') {
    addon.init(true)
    addon.openChannel(testCid, addon.CHAN_R, 12)
    // generate test data
    let buf = new Uint8Array(dataSz)
    // send data
    addon.printChannelStatus(testCid)
    let n = addon.readChannel(testCid, buf, dataSz, true)
    console.log('read ' + n)
    addon.printChannelStatus(testCid)
    // clean
    addon.closeChannel(testCid)
    addon.deinit()
    // print test data
    let s = ''
    for (let i = 0; i < dataSz; i++) {
        s += buf[i] + ' '
    }
    console.log(s)
} else if (args[0] == '-w') {
    addon.init(true)
    addon.openChannel(testCid, addon.CHAN_W, 12)
    // generate test data
    let buf = new Uint8Array(dataSz)
    for (let i = 0; i < dataSz; i++) {
        buf[i] = Math.round(Math.random() * 255)
    }
    // send data
    addon.printChannelStatus(testCid)
    addon.writeChannel(testCid, buf, dataSz)
    addon.printChannelStatus(testCid)
    // clean
    addon.closeChannel(testCid)
    addon.deinit()
    // print test data
    let s = ''
    for (let i = 0; i < dataSz; i++) {
        s += buf[i] + ' '
    }
    console.log(s)
} else {
    console.error('Invalid option.')
}
