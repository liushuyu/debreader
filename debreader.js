const addon = require('bindings')('addon');
addon.parse('test.deb', (err, res) => {console.log(res)});
