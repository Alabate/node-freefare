#include "device.h"

extern "C" {
#include <nfc/nfc.h>
}

#define MAX_CANDIDATES 16

using namespace Nan;

Device::Device(std::string connstring) : connstring(connstring) {}
Device::~Device() {}


NAN_MODULE_INIT(Device::Init) {
	v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(Device::New);
	tpl->SetClassName(Nan::New("Device").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	Nan::SetPrototypeMethod(tpl, "open", Device::Open);
	Nan::SetPrototypeMethod(tpl, "listTags", Device::ListTags);
	Nan::SetPrototypeMethod(tpl, "poll", Device::Poll);
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
	Nan::EscapableHandleScope scope;

	v8::Local<v8::Value> argv[1] = { Nan::New<v8::String>(connstring).ToLocalChecked() };

	v8::Local<v8::Function> cons = Nan::New(constructor());
	return scope.Escape(Nan::NewInstance(cons, 1, argv).ToLocalChecked());
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

int FreefareList(nfc_device *device, FreefareTag &tag) {
	nfc_initiator_init(device);
	// Disabling NP_AUTO_ISO14443_4 saves a massive amount of time. ~400ms.
	nfc_device_set_property_bool(device, NP_AUTO_ISO14443_4, false);

	// Poll infinitely
	const uint8_t uiPollNr = 0xff;
	// Period in increments of 150ms. So poll every 150ms.
	const uint8_t uiPeriod = 1;
	const nfc_modulation nmModulations[1] = {
		{ .nmt = NMT_ISO14443A, .nbr = NBR_106 }
	};
	const size_t szModulations = sizeof(nmModulations) / sizeof(nfc_modulation);
	nfc_target nt;

	int res = 0;
	if ((res = nfc_initiator_poll_target(device, nmModulations, szModulations, uiPollNr, uiPeriod, &nt)) < 0) {
		nfc_perror(device, "nfc_initiator_poll_target");
		return res;
	}

	if (res > 0) {
		tag = freefare_tag_new(device, nt);
		return res;
	}

	return 0;
}

NAN_METHOD(Device::Poll){
	Device *obj = ObjectWrap::Unwrap<Device>(info.This());
	Callback *callback = new Callback(info[0].As<v8::Function>());

	AsyncQueueWorker(new AsyncWrapper(callback, [obj]() {
		FreefareTag tag;
		int res = FreefareList(obj->device, tag);
		return [res, tag](AsyncWrapper &wrapper) {
			v8::Local<v8::Value> err = Nan::Null();
			v8::Local<v8::Value> result = Nan::Null();
			if (res < 0) {
				err = Nan::New<v8::Number>(res);
			} else if (res > 0) {
				result = Tag::Instantiate(tag);
			}
			v8::Local<v8::Value> argv[] = {
				err,
				result
			};
			wrapper.SetArgs(2, argv);
		};
	}));
}
