
const path = require('path')
const lib = require('./build/Release/smipc.node')
lib.loadSmipc(path.join(__dirname, 'lib/libsmipc.dll'))
delete lib['loadSmipc']
module.exports = lib
