#ifndef NFF_FREEFARE_H
#define NFF_FREEFARE_H

#include <nan.h>
#include <string>

extern "C" {
#include <nfc/nfc.h>
#include <freefare.h>
}

#include "tag.h"
#include "device.h"
#include "common.h"

class Freefare: public Nan::ObjectWrap {

	public:
		static NAN_MODULE_INIT(Init);

	private:
		explicit Freefare();
		~Freefare();

        static inline Nan::Persistent<v8::Function> & constructor();

        static NAN_METHOD(New);
		static NAN_METHOD(InitLibNFC);
		static NAN_METHOD(ListDevices);
};


#endif /* NFF_FREEFARE_H */
