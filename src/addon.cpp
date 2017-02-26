#include <nan.h>
#include "deb_info_nan.h"

// Exported to JS world
static void parse(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  v8::Local<v8::String> opts = info[0].As<v8::String>();
  bool ls = false;
  if (info.Length() > 1) {
    ls = info[1]->BooleanValue();
  }
  std::string fn(*v8::String::Utf8Value(opts->ToString()));
  Nan::Callback *callback = new Nan::Callback(info[1].As<v8::Function>());
  Nan::AsyncQueueWorker(new DebReader(callback, fn, ls));
  return;
}

NAN_MODULE_INIT(init) {
  Nan::Set(target, Nan::New("parse").ToLocalChecked(),
    Nan::GetFunction(Nan::New<v8::FunctionTemplate>(parse)).ToLocalChecked());
}

NODE_MODULE(addon, init)
