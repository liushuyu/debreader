'use strict';
(() => {
  const addon = require('bindings')('addon');
  const path = require('path');
  const fs = require('fs');

  function parseControl(content, callback) {
    var control = content.trim().split('\n');
    var parsed = {};
    for (let item of control) {
      let key, value;
      [key, value] = item.split(': ');
      parsed[key] = value;
    }
    callback(parsed);
  }

  exports.read = (filename, callback, opts = {}) => {
    if (typeof callback !== 'function') {
      throw new TypeError('Callback is not a function');
    }
    fs.access(filename, fs.constants.R_OK, (err) => {
      if (err) {
        callback(err, null);
      }
      addon.parse(filename, (err, res) => {
        if (err) {
          callback(err, null);
          return;
        }
        if (!opts.parse) {
          res.controlFile = res.controlFile.trim();
          callback(err, res);
          return;
        }
        parseControl(res.controlFile, (result) => {
          res.controlFile = result;
          callback(err, res);
        }, opts.controlOnly);
      });
    });
  }

})();
