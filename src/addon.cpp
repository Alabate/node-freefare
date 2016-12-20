#include "device.h"
#include "freefare.h"
#include "common.h"

// Global vars init
nfc_context* libnfc_context = NULL;


NAN_MODULE_INIT(Init) {
    Freefare::Init(target);
	Device::Init(target);
	Tag::Init(target);
}

NODE_MODULE(freefare, Init)
