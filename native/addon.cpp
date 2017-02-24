#include <nan.h>
#include "deb_info_nan.h"

NAN_MODULE_INIT(init) {
  Nan::Set(target, Nan::New("parse").ToLocalChecked(),
    Nan::GetFunction(Nan::New<v8::FunctionTemplate>(parse)).ToLocalChecked());
}

NODE_MODULE(addon, init)
