#include "device.h"

using namespace Nan;

Device::Device(std::string connstring) : connstring(connstring) {}
Device::~Device() {}


NAN_MODULE_INIT(Device::Init) {
	v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(Device::New);
	tpl->SetClassName(Nan::New("Device").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	Nan::SetPrototypeMethod(tpl, "open", Device::Open);
	Nan::SetPrototypeMethod(tpl, "listTags", Device::ListTags);
	Nan::SetPrototypeMethod(tpl, "getConnstring", Device::GetConnstring);
	Nan::SetPrototypeMethod(tpl, "abort", Device::Abort);

	constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
	Nan::Set(target, Nan::New("Device").ToLocalChecked(),
	Nan::GetFunction(tpl).ToLocalChecked());
}

Nan::Persistent<v8::Function> & Device::constructor() {
	static Nan::Persistent<v8::Function> my_constructor;
	return my_constructor;
}

NAN_METHOD(Device::New) {
	if (info.IsConstructCall()) {
		std::string connstring = std::string(*v8::String::Utf8Value(info[0]->ToString()));
		Device *obj = new Device(connstring);
		obj->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
	} else {
		const int argc = 1;
		v8::Local<v8::Value> argv[argc] = {info[0]};
		v8::Local<v8::Function> cons = Nan::New(constructor());
		info.GetReturnValue().Set(Nan::NewInstance(cons, argc, argv).ToLocalChecked());
	}
}

v8::Handle<v8::Value> Device::Instantiate(std::string connstring) {
	v8::Local<v8::Value> argv[1] = { Nan::New<v8::String>(connstring).ToLocalChecked() };

	v8::Local<v8::Function> cons = Nan::New(constructor());
	return Nan::NewInstance(cons, 1, argv).ToLocalChecked();
}

/**
* OpenDevice
*/
class OpenWorker : public AsyncWorker {
public:
	OpenWorker(Callback *callback, std::string connstring, nfc_device **devicecde)
	: AsyncWorker(callback), connstring(connstring), deviceabc(devicecde) {}

	~OpenWorker() {}

	void Execute () {
		// open Device
		*deviceabc = nfc_open(libnfc_context, connstring.c_str());
	}

	void HandleOKCallback () {
		Nan:: HandleScope scope;

		// Return error or null
		v8::Local<v8::Value> err = Null();
		if(!*deviceabc) {
			err = New<v8::Number>(NFF_ERROR_OPEN_DEVICE);
		}

		v8::Local<v8::Value> argv[] = {
			err
		};

		callback->Call(1, argv);
	}


private:

	// Description of the connexion to the device
	std::string connstring;

	// LibNFC device
	nfc_device** deviceabc;
};
NAN_METHOD(Device::Open) {
	Device* obj = ObjectWrap::Unwrap<Device>(info.This());

	Callback *callback = new Callback(info[0].As<v8::Function>());
	AsyncQueueWorker(new OpenWorker(callback, obj->connstring, &(obj->device)));
}

/**
* CloseDevice
*/
class CloseWorker : public AsyncWorker {
public:
	CloseWorker(Callback *callback, nfc_device *device)
	: AsyncWorker(callback), device(device) {}
	~CloseWorker() {}

	void Execute () {
		nfc_close(device);
	}

	void HandleOKCallback () {
		Nan:: HandleScope scope;

		v8::Local<v8::Value> argv[] = {
			Null()
		};

		callback->Call(1, argv);
	}

private:

	// LibNFC device
	nfc_device* device;
};
NAN_METHOD(Device::Close) {
	Device* obj = ObjectWrap::Unwrap<Device>(info.This());

	Callback *callback = new Callback(info[0].As<v8::Function>());
	AsyncQueueWorker(new CloseWorker(callback, obj->device));
}


NAN_METHOD(Device::GetConnstring) {
	Device* obj = ObjectWrap::Unwrap<Device>(info.This());

	info.GetReturnValue().Set(Nan::New<v8::String>(obj->connstring).ToLocalChecked());
}



class ListTagsWorker : public AsyncWorker {
public:
	ListTagsWorker(Callback *callback, nfc_device *devicecde)
	: AsyncWorker(callback), deviceabc(devicecde) {}

	~ListTagsWorker() {}

	void Execute () {
		// open Device
		tags = freefare_get_tags(deviceabc);
	}

	void HandleOKCallback () {
		Nan:: HandleScope scope;

		v8::Local<v8::Value> err = Null();

		// Find number of tags
		size_t count = 0;
		while(tags[count]) {
			count++;
		}

		// Return tags objects
		v8::Local<v8::Array> results = New<v8::Array>(count);
		for (size_t i = 0; i < count; i++) {
			v8::Local<v8::Value> tmp = Tag::Instantiate(tags[i]);
			Nan::Set(results, i, tmp);
		}

		v8::Local<v8::Value> argv[] = {
			err,
			results
		};

		callback->Call(2, argv);
	}


private:

	// LibNFC device
	nfc_device* deviceabc;

	// Found tags
	MifareTag* tags;
};
NAN_METHOD(Device::ListTags) {
	Device* obj = ObjectWrap::Unwrap<Device>(info.This());

	Callback *callback = new Callback(info[0].As<v8::Function>());
	AsyncQueueWorker(new ListTagsWorker(callback, obj->device));
}

/**
* Abort current function
*/
class AbortWorker : public AsyncWorker {
public:
	AbortWorker(Callback *callback, nfc_device *devicecde)
	: AsyncWorker(callback), deviceabc(devicecde), error(0) {}

	~AbortWorker() {}

	void Execute () {
		error = nfc_abort_command(deviceabc);
	}

	void HandleOKCallback () {
		v8::Local<v8::Value> argv[] = {
			New<v8::Number>(error)
		};

		callback->Call(2, argv);
	}
private:
	// LibNFC device
	nfc_device* deviceabc;

	// Error ID or 0
	int error;

};
NAN_METHOD(Device::Abort) {
	Device* obj = ObjectWrap::Unwrap<Device>(info.This());

	Callback *callback = new Callback(info[0].As<v8::Function>());
	AsyncQueueWorker(new AbortWorker(callback, obj->device));
}
