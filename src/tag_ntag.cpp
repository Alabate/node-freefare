#include "tag.h"

#include <functional>

using namespace Nan;

#define READ_WRITE_BYTES (4) // Number of bytes to read/write at a time.
#define PASSWORD_LENGTH (4) // Length of password in bytes.
#define PACK_LENGTH (2) // Password acknowledgement length in bytes.

NAN_METHOD(Tag::ntag_connect) {
	Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
	Callback *callback = new Callback(info[0].As<v8::Function>());
	AsyncQueueWorker(new AsyncWrapper(callback, [obj]() {
		int error = ntag21x_connect(obj->tag);
		return [error](AsyncWrapper &wrapper) {
			v8::Local<v8::Value> argv[] = {
				Nan::New<v8::Number>(error)
			};
			wrapper.SetArgs(1, argv);
		};
	}));
}

NAN_METHOD(Tag::ntag_disconnect) {
	Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
	Callback *callback = new Callback(info[0].As<v8::Function>());
	AsyncQueueWorker(new AsyncWrapper(callback, [obj]() {
		int error = ntag21x_disconnect(obj->tag);
		return [error](AsyncWrapper &wrapper) {
			v8::Local<v8::Value> argv[] = {
				Nan::New<v8::Number>(error)
			};
			wrapper.SetArgs(1, argv);
		};
	}));
}

NAN_METHOD(Tag::ntag_getInfo) {
	Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
	Callback *callback = new Callback(info[0].As<v8::Function>());
	AsyncQueueWorker(new AsyncWrapper(callback, [obj]() {
		int error = ntag21x_get_info(obj->tag);
		return [error](AsyncWrapper &wrapper) {
			v8::Local<v8::Value> argv[] = {
				Nan::New<v8::Number>(error)
			};
			wrapper.SetArgs(1, argv);
		};
	}));
}

NAN_METHOD(Tag::ntag_getType) {
	Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
	Callback *callback = new Callback(info[0].As<v8::Function>());
	AsyncQueueWorker(new AsyncWrapper(callback, [obj]() {
		ntag_tag_subtype type = ntag21x_get_subtype(obj->tag);
		std::string typeStr = "Unknown";
		switch (type) {
			case NTAG_213:
				typeStr = "NTAG213";
				break;
			case NTAG_215:
				typeStr = "NTAG215";
				break;
			case NTAG_216:
				typeStr = "NTAG216";
				break;
			default:
				break;
		}
		return [typeStr](AsyncWrapper &wrapper) {
			v8::Local<v8::Value> argv[] = {
				Nan::New<v8::Number>(0),
				Nan::New<v8::String>(typeStr).ToLocalChecked()
			};
			wrapper.SetArgs(2, argv);
		};
	}));
}

NAN_METHOD(Tag::ntag_read) {
	Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
	Callback *callback = new Callback(info[1].As<v8::Function>());
	uint32_t page = info[0]->Uint32Value();
	AsyncQueueWorker(new AsyncWrapper(callback, [page, obj]() {
		uint8_t *data = new uint8_t[READ_WRITE_BYTES];
		int error = ntag21x_fast_read4(obj->tag, page, data);
		return [data, error](AsyncWrapper &wrapper) {
			Nan::MaybeLocal<v8::Object> buf =  Nan::CopyBuffer(reinterpret_cast<char*>(data), 4);
			delete[] data;
			v8::Local<v8::Value> argv[] = {
				Nan::New<v8::Number>(error),
				buf.ToLocalChecked()
			};
			wrapper.SetArgs(2, argv);
		};
	}));
}

NAN_METHOD(Tag::ntag_write) {
	Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
	uint32_t page = info[0]->Uint32Value();
	uint8_t *origBuffer = reinterpret_cast<unsigned char*>(node::Buffer::Data(info[1]));
	uint8_t *buffer = new uint8_t[READ_WRITE_BYTES];
	memcpy(buffer, origBuffer, READ_WRITE_BYTES);
	Callback *callback = new Callback(info[2].As<v8::Function>());
	AsyncQueueWorker(new AsyncWrapper(callback, [obj, page, buffer]() {
		int error = ntag21x_write(obj->tag, page, buffer);
		delete[] buffer;
		return [error](AsyncWrapper &wrapper) {
			v8::Local<v8::Value> argv[] = {
				Nan::New<v8::Number>(error)
			};
			wrapper.SetArgs(1, argv);
		};
	}));
}

NAN_METHOD(Tag::ntag_set_auth) {
	Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());

	uint8_t *origBuffer = reinterpret_cast<unsigned char*>(node::Buffer::Data(info[0]));
	uint8_t *password = new uint8_t[PASSWORD_LENGTH];
	memcpy(password, origBuffer, PASSWORD_LENGTH);

	uint8_t *origPack = reinterpret_cast<unsigned char*>(node::Buffer::Data(info[1]));
	uint8_t *pack = new uint8_t[PACK_LENGTH];
	memcpy(pack, origPack, PACK_LENGTH);

	// Verified range in freefare.js
	uint8_t startPage = static_cast<uint8_t>(info[2]->Uint32Value());
	bool prot = info[3]->BooleanValue();

	Callback *callback = new Callback(info[4].As<v8::Function>());

	AsyncQueueWorker(new AsyncWrapper(callback, [obj, password, pack, startPage, prot]() {
		NTAG21xKey key = ntag21x_key_new(password, pack);

		// First disable auth in case set key fails.
		// Set key may fail by setting the password but not the pack.
		int error = ntag21x_set_auth(obj->tag, startPage);
		if (error < 0) {
			error = -1;
			goto end;
		}

		error = ntag21x_set_key(obj->tag, key);
		if (error < 0) {
			error = -2;
			goto end;
		}

		// Authenticate to ensure password and pack are set correctly.
		error = ntag21x_authenticate(obj->tag, key);
		if (error < 0) {
			error = -3;
			goto end;
		}

		ntag21x_key_free(key);

		error = ntag21x_set_auth(obj->tag, startPage);
		if (error < 0) {
			error = -4;
			goto end;
		}

		if (prot) {
			error = ntag21x_access_enable(obj->tag, NTAG_PROT);
		} else {
			error = ntag21x_access_disable(obj->tag, NTAG_PROT);
		}
		if (error < 0) {
			error = -5;
		}

		end:
		delete[] password;
		delete[] pack;
		return [error](AsyncWrapper &wrapper) {
			v8::Local<v8::Value> argv[] = {
				Nan::New<v8::Number>(error)
			};
			wrapper.SetArgs(1, argv);
		};
	}));
}

NAN_METHOD(Tag::ntag_disable_auth) {
	Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
	Callback *callback = new Callback(info[0].As<v8::Function>());

	AsyncQueueWorker(new AsyncWrapper(callback, [obj]() {
		int error = ntag21x_set_auth(obj->tag, 0xff);
		if (error < 0) {
			error = -1;
			goto end;
		}

		error = ntag21x_access_disable(obj->tag, NTAG_PROT);
		if (error < 0) {
			error = -2;
		}

		end:
		return [error](AsyncWrapper &wrapper) {
			v8::Local<v8::Value> argv[] = {
				Nan::New<v8::Number>(error)
			};
			wrapper.SetArgs(1, argv);
		};
	}));
}

NAN_METHOD(Tag::ntag_authenticate) {
	Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
	
	uint8_t *origBuffer = reinterpret_cast<unsigned char*>(node::Buffer::Data(info[0]));
	uint8_t *password = new uint8_t[PASSWORD_LENGTH];
	memcpy(password, origBuffer, PASSWORD_LENGTH);

	uint8_t *origPack = reinterpret_cast<unsigned char*>(node::Buffer::Data(info[1]));
	uint8_t *pack = new uint8_t[PACK_LENGTH];
	memcpy(pack, origPack, PACK_LENGTH);

	Callback *callback = new Callback(info[2].As<v8::Function>());

	AsyncQueueWorker(new AsyncWrapper(callback, [obj, password, pack]() {
		NTAG21xKey key = ntag21x_key_new(password, pack);
		int error = ntag21x_authenticate(obj->tag, key);
		ntag21x_key_free(key);

		delete[] password;
		delete[] pack;
		return [error](AsyncWrapper &wrapper) {
			v8::Local<v8::Value> argv[] = {
				Nan::New<v8::Number>(error)
			};
			wrapper.SetArgs(1, argv);
		};
	}));
}
