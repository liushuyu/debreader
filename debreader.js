'use strict';
(() => {
  const addon = require('bindings')('addon');
  const path = require('path');
  const fs = require('fs');
  exports.read = (filename, callback) => {
    if (typeof callback !== 'function') {
      throw new TypeError('Callback is not a function');
    }
    fs.access(filename, fs.constants.R_OK, (err) => {
      if (err) {callback(err, null);}
      addon.parse(path.resolve(filename), (err, res) => {
        callback(err, res);
      });
    });
  }
})();
