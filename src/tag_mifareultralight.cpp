#include "tag.h"

using namespace Nan;


class mifareUltralight_connectWorker : public AsyncWorker {
public:
	mifareUltralight_connectWorker(Callback *callback, MifareTag tag)
	: AsyncWorker(callback), tag(tag), error(0) {}
	~mifareUltralight_connectWorker() {}

	void Execute () {
		error = mifare_ultralight_connect(tag);
	}

	void HandleOKCallback () {
		Nan::HandleScope scope;

		v8::Local<v8::Value> argv[] = {
			New<v8::Number>(error)
		};

		callback->Call(1, argv);
	}
private:

	// Our current tag
	MifareTag tag;

	// Error ID or 0
	int error;

};
NAN_METHOD(Tag::mifareUltralight_connect) {
	Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
	Callback *callback = new Callback(info[0].As<v8::Function>());
	AsyncQueueWorker(new mifareUltralight_connectWorker(callback, obj->tag));
}


class mifareUltralight_disconnectWorker : public AsyncWorker {
public:
	mifareUltralight_disconnectWorker(Callback *callback, MifareTag tag)
	: AsyncWorker(callback), tag(tag), error(0) {}
	~mifareUltralight_disconnectWorker() {}

	void Execute () {
		error = mifare_ultralight_disconnect(tag);
	}

	void HandleOKCallback () {
		Nan::HandleScope scope;

		v8::Local<v8::Value> argv[] = {
			New<v8::Number>(error)
		};

		callback->Call(1, argv);
	}
private:

	// Our current tag
	MifareTag tag;

	// Error ID or 0
	int error;

};
NAN_METHOD(Tag::mifareUltralight_disconnect) {
	Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
	Callback *callback = new Callback(info[0].As<v8::Function>());
	AsyncQueueWorker(new mifareUltralight_disconnectWorker(callback, obj->tag));
}


class mifareUltralight_readWorker : public AsyncWorker {
public:
	mifareUltralight_readWorker(Callback *callback, MifareTag tag, MifareUltralightPageNumber page)
	: AsyncWorker(callback), tag(tag), page(page), error(0) {}
	~mifareUltralight_readWorker() {}

	void Execute () {
		error = mifare_ultralight_read(tag, page, &data);
	}

	void HandleOKCallback () {
		Nan::HandleScope scope;

		Nan::MaybeLocal<v8::Object> buf =  Nan::CopyBuffer(reinterpret_cast<char*>(data), sizeof(data));

		v8::Local<v8::Value> argv[] = {
			New<v8::Number>(error),
			buf.ToLocalChecked()
		};

		callback->Call(2, argv);
	}
private:

	// Our current tag
	MifareTag tag;

	// Page to read
	MifareUltralightPageNumber page;

	// Page content
	MifareUltralightPage data;

	// Error ID or 0
	int error;

};
NAN_METHOD(Tag::mifareUltralight_read) {
	Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
	Callback *callback = new Callback(info[1].As<v8::Function>());
	AsyncQueueWorker(new mifareUltralight_readWorker(callback, obj->tag, info[0]->Uint32Value()));
}


class mifareUltralight_writeWorker : public AsyncWorker {
public:
	mifareUltralight_writeWorker(Callback *callback, MifareTag tag, MifareUltralightPageNumber page, MifareUltralightPage data)
	: AsyncWorker(callback), tag(tag), page(page), error(0) {
		memcpy(this->data, data, sizeof(this->data));
	}
	~mifareUltralight_writeWorker() {}

	void Execute () {
		error = mifare_ultralight_write(tag, page, data);
	}

	void HandleOKCallback () {
		Nan::HandleScope scope;

		v8::Local<v8::Value> argv[] = {
			New<v8::Number>(error)
		};

		callback->Call(1, argv);
	}
private:

	// Our current tag
	MifareTag tag;

	// Page to read
	MifareUltralightPageNumber page;

	// Read content
	MifareUltralightPage data;

	// Error ID or 0
	int error;

};
NAN_METHOD(Tag::mifareUltralight_write) {
	Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
	Callback *callback = new Callback(info[2].As<v8::Function>());
	AsyncQueueWorker(new mifareUltralight_writeWorker(callback, obj->tag, info[0]->Uint32Value(), reinterpret_cast<unsigned char*>(node::Buffer::Data(info[1]))));
}
