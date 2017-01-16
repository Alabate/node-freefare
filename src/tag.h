#ifndef NFF_TAG_H
#define NFF_TAG_H

#include <nan.h>
#include <string>

extern "C" {
	#include <nfc/nfc.h>
	#include <freefare.h>
}

#include "common.h"
#include "device.h"




class Tag: public Nan::ObjectWrap {

public:
	static NAN_MODULE_INIT(Init);
	static v8::Handle<v8::Value> Instantiate(MifareTag tag);

private:
	explicit Tag(MifareTag tag);
	~Tag();

	static inline Nan::Persistent<v8::Function> & constructor();

	static NAN_METHOD(New);
	static NAN_METHOD(GetTagType);
	static NAN_METHOD(GetTagFriendlyName);
	static NAN_METHOD(GetTagUID);

	static NAN_METHOD(mifareUltralight_connect);
	static NAN_METHOD(mifareUltralight_disconnect);
	static NAN_METHOD(mifareUltralight_read);
	static NAN_METHOD(mifareUltralight_write);

	static NAN_METHOD(mifareClassic_connect);
	static NAN_METHOD(mifareClassic_disconnect);
	static NAN_METHOD(mifareClassic_authenticate);
	static NAN_METHOD(mifareClassic_read);
	static NAN_METHOD(mifareClassic_initValue);
	static NAN_METHOD(mifareClassic_readValue);
	static NAN_METHOD(mifareClassic_write);
	static NAN_METHOD(mifareClassic_increment);
	static NAN_METHOD(mifareClassic_decrement);
	static NAN_METHOD(mifareClassic_restore);
	static NAN_METHOD(mifareClassic_transfer);

	static NAN_METHOD(mifareDesfire_connect);
	static NAN_METHOD(mifareDesfire_disconnect);
	static NAN_METHOD(mifareDesfire_authenticate_des);
	static NAN_METHOD(mifareDesfire_authenticate_3des);
	static NAN_METHOD(mifareDesfire_getApplicationIds);
	static NAN_METHOD(mifareDesfire_selectApplication);
	static NAN_METHOD(mifareDesfire_getFileIds);
	static NAN_METHOD(mifareDesfire_write);
	static NAN_METHOD(mifareDesfire_read);


private:
	nfc_device* device;
	std::string connstring;
	MifareTag tag;



	static MifareTag constructorTag;
};


#endif /* NFF_TAG_H */
