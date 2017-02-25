const debreader = require('./debreader');

describe('Basic Tests', function() {
  it('should read good packages', function(done) {
    debreader.read('tests/good_deb.bin', (err, res) => {
      if (err) throw err;
      done();
    });
  });
  it('should return error when reading broken packages', (done) => {
    debreader.read('tests/broken_deb.bin', (err, res) => {
      if (!err) throw new Error('WTF? It can\'t be!');
      done();
    });
  });
  it('should return error when reading non-debian packages', (done) => {
    debreader.read('tests/bad_deb.bin', (err, res) => {
      if (!err) throw new Error('WTF? It can\'t be!');
      done();
    });
  });
});
