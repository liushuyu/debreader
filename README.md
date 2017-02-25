### DebReader
A small Javascript library (With native extensions, for Node.JS of course) for reading/extracting information Debian software packages `(*.deb)` files. Licensed under MIT License.

##### Install
You need to have following packages/softwares installed:

1. Node.JS
2. NPM (Node Package Manager, usually comes with Node.JS installs)
3. libarchive (With development files. If you can't find it in your distribution, it may come with another name like `bsdtar`. If your distribution split packages, you need to install `libarchive-dev` or similar)
4. C++ 11 compatible compilers (Like `gcc`, `clang`)

When you have everything above installed, then use `npm` to install this package: `npm install deb-reader --save`

##### Usage
In your project:
```Javascript
const debreader = require('debreader');
debreader.read('something.deb', function (err, res) {
  // Do something...
});
```
