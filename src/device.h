#ifndef NFF_DEVICE_H
#define NFF_DEVICE_H

#include <nan.h>
#include <string>

extern "C" {
#include <nfc/nfc.h>
#include <freefare.h>
}

#include "common.h"
#include "tag.h"




class Device: public Nan::ObjectWrap {

	public:
		static NAN_MODULE_INIT(Init);
		static v8::Handle<v8::Value> Instantiate(std::string connstring);

	private:
		explicit Device(std::string connstring);
		~Device();

		static inline Nan::Persistent<v8::Function> & constructor();

		static NAN_METHOD(New);
		static NAN_METHOD(Open);
		static NAN_METHOD(Close);
		static NAN_METHOD(GetConnstring);
		static NAN_METHOD(ListTags);
		static NAN_METHOD(Abort);


	private:
		nfc_device* device;
		std::string connstring;
};


#endif /* NFF_DEVICE_H */
