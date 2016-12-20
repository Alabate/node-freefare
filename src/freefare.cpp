#include "freefare.h"


using namespace Nan;

Freefare::Freefare() {}
Freefare::~Freefare() {}

NAN_MODULE_INIT(Freefare::Init) {

    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(Freefare::New);
    tpl->SetClassName(Nan::New("Freefare").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "init", Freefare::InitLibNFC);
    Nan::SetPrototypeMethod(tpl, "listDevices", Freefare::ListDevices);

    constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("Freefare").ToLocalChecked(),
        Nan::GetFunction(tpl).ToLocalChecked());
}

Nan::Persistent<v8::Function> & Freefare::constructor() {
    static Nan::Persistent<v8::Function> my_constructor;
    return my_constructor;
}

NAN_METHOD(Freefare::New) {
  if (info.IsConstructCall()) {
    info.GetReturnValue().Set(info.This());
  } else {
    const int argc = 0;
    v8::Local<v8::Value> argv[argc] = {};
    v8::Local<v8::Function> cons = Nan::New(constructor());
    info.GetReturnValue().Set(Nan::NewInstance(cons, argc, argv).ToLocalChecked());
  }
}

/**
 * Init LibNFC
 */
NAN_METHOD(Freefare::InitLibNFC) {
    if(libnfc_context != NULL) {
        nfc_exit(libnfc_context);
    }

    nfc_init(&libnfc_context);
    if (libnfc_context == NULL) {
        info.GetReturnValue().Set(Nan::New<v8::Number>(NFF_ERROR_INIT_LIBNFC));
    }
    info.GetReturnValue().Set(Null());
}

/**
 * List devices
 */
class ListDevicesWorker : public AsyncWorker {
    public:
        ListDevicesWorker(Callback *callback)
        : AsyncWorker(callback), devices(0), error(0) {}

        ~ListDevicesWorker() {}

        void Execute () {
            nfc_connstring deviceList[NFF_MAX_DEVICES];
            size_t deviceCount;

            // List all devices
            deviceCount = nfc_list_devices(libnfc_context, deviceList, NFF_MAX_DEVICES);
            for (size_t d = 0; d < deviceCount; d++) {
                devices.push_back(std::string(deviceList[d]));
            }
        }

        void HandleOKCallback () {
            Nan::HandleScope scope;

            v8::Local<v8::Array> results = New<v8::Array>(devices.size());
            int i = 0;
            for_each(devices.begin(), devices.end(),
                [&](std::string connstring) {
                    Nan::Set(results, i, Device::Instantiate(connstring));
                    i++;
            });

            v8::Local<v8::Value> argv[] = {
                New<v8::Number>(error),
                results
            };

            callback->Call(2, argv);
        }
    private:


        // List of nfc_device we manage to open
        std::vector<std::string> devices;

        // Error ID or 0
        int error;

};
NAN_METHOD(Freefare::ListDevices) {
    Callback *callback = new Callback(info[0].As<v8::Function>());
    AsyncQueueWorker(new ListDevicesWorker(callback));
}
