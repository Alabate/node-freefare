#include "tag.h"

using namespace Nan;


class mifareClassic_connectWorker : public AsyncWorker {
    public:
        mifareClassic_connectWorker(Callback *callback, MifareTag tag)
        : AsyncWorker(callback), tag(tag), error(0) {}
        ~mifareClassic_connectWorker() {}

        void Execute () {
            error = mifare_classic_connect(tag);
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
NAN_METHOD(Tag::mifareClassic_connect) {
    Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
    Callback *callback = new Callback(info[0].As<v8::Function>());
    AsyncQueueWorker(new mifareClassic_connectWorker(callback, obj->tag));
}


class mifareClassic_disconnectWorker : public AsyncWorker {
    public:
        mifareClassic_disconnectWorker(Callback *callback, MifareTag tag)
        : AsyncWorker(callback), tag(tag), error(0) {}
        ~mifareClassic_disconnectWorker() {}

        void Execute () {
            error = mifare_classic_disconnect(tag);
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
NAN_METHOD(Tag::mifareClassic_disconnect) {
    Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
    Callback *callback = new Callback(info[0].As<v8::Function>());
    AsyncQueueWorker(new mifareClassic_disconnectWorker(callback, obj->tag));
}

class mifareClassic_authenticateWorker : public AsyncWorker {
    public:
        mifareClassic_authenticateWorker(Callback *callback, MifareTag tag, const MifareClassicBlockNumber block, const MifareClassicKey key, const MifareClassicKeyType keyType)
        : AsyncWorker(callback), tag(tag), block(block), keyType(keyType), error(0) {
            memcpy(this->key, key, sizeof(MifareClassicKey));
        }
        ~mifareClassic_authenticateWorker() {}

        void Execute () {
            error = mifare_classic_authenticate(tag, block, key, keyType);
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

        // Block to read
        MifareClassicBlockNumber block;

        // Key
    	MifareClassicKey key;

        // Key type (MFC_KEY_A or MFC_KEY_B)
    	MifareClassicKeyType keyType;

        // Error ID or 0
        int error;

};
NAN_METHOD(Tag::mifareClassic_authenticate) {
    Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());

    AsyncQueueWorker(new mifareClassic_authenticateWorker(
        new Callback(info[3].As<v8::Function>()),
        obj->tag,
        info[0]->Uint32Value(),
        reinterpret_cast<unsigned char*>(node::Buffer::Data(info[1])),
        (std::string(*v8::String::Utf8Value(info[2]->ToString())) == "A") ? MFC_KEY_A : MFC_KEY_B
    ));
}

class mifareClassic_readWorker : public AsyncWorker {
    public:
        mifareClassic_readWorker(Callback *callback, MifareTag tag, MifareClassicBlockNumber block)
        : AsyncWorker(callback), tag(tag), block(block), error(0) {}
        ~mifareClassic_readWorker() {}

        void Execute () {
            error = mifare_classic_read(tag, block, &data);
        }

        void HandleOKCallback () {
            Nan::HandleScope scope;

            Nan::MaybeLocal<v8::Object> buf =  Nan::CopyBuffer(reinterpret_cast<char*>(&data), sizeof(MifareClassicBlock));

            v8::Local<v8::Value> argv[] = {
                New<v8::Number>(error),
                buf.ToLocalChecked()
            };

            callback->Call(2, argv);
        }
    private:

        // Our current tag
    	MifareTag tag;

        // Block to read
        MifareClassicBlockNumber block;

        // Block content
    	MifareClassicBlock data;

        // Error ID or 0
        int error;

};
NAN_METHOD(Tag::mifareClassic_read) {
    Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
    Callback *callback = new Callback(info[1].As<v8::Function>());
    AsyncQueueWorker(new mifareClassic_readWorker(callback, obj->tag, info[0]->Uint32Value()));
}


class mifareClassic_initValueWorker : public AsyncWorker {
    public:
        mifareClassic_initValueWorker(Callback *callback, MifareTag tag, MifareClassicBlockNumber block, const int32_t value, const MifareClassicBlockNumber adr)
        : AsyncWorker(callback), tag(tag), block(block), value(value), adr(adr), error(0) {}
        ~mifareClassic_initValueWorker() {}

        void Execute () {
            error = mifare_classic_init_value(tag, block, value, adr);
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

        // Block to read
        MifareClassicBlockNumber block;
        int32_t value;
        MifareClassicBlockNumber adr;

        // Error ID or 0
        int error;

};
NAN_METHOD(Tag::mifareClassic_initValue) {
    Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
    Callback *callback = new Callback(info[3].As<v8::Function>());
    AsyncQueueWorker(new mifareClassic_initValueWorker(callback, obj->tag, info[0]->Uint32Value(), info[1]->Int32Value(), info[2]->Uint32Value()));
}


class mifareClassic_readValueWorker : public AsyncWorker {
    public:
        mifareClassic_readValueWorker(Callback *callback, MifareTag tag, MifareClassicBlockNumber block)
        : AsyncWorker(callback), tag(tag), block(block), error(0) {}
        ~mifareClassic_readValueWorker() {}

        void Execute () {
            error = mifare_classic_read_value(tag, block, &value, &adr);
        }

        void HandleOKCallback () {
            Nan::HandleScope scope;

            v8::Local<v8::Value> argv[] = {
                New<v8::Number>(error),
                New<v8::Number>(value),
                New<v8::Number>(adr)
            };

            callback->Call(3, argv);
        }
    private:

        // Our current tag
    	MifareTag tag;

        // Block to read
        MifareClassicBlockNumber block;
        int32_t value;
        MifareClassicBlockNumber adr;

        // Error ID or 0
        int error;

};
NAN_METHOD(Tag::mifareClassic_readValue) {
    Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
    Callback *callback = new Callback(info[1].As<v8::Function>());
    AsyncQueueWorker(new mifareClassic_readValueWorker(callback, obj->tag, info[0]->Uint32Value()));
}



class mifareClassic_writeWorker : public AsyncWorker {
    public:
        mifareClassic_writeWorker(Callback *callback, MifareTag tag, MifareClassicBlockNumber block, const MifareClassicBlock data)
        : AsyncWorker(callback), tag(tag), block(block), error(0) {
            memcpy(this->data, data, sizeof(MifareClassicBlock));
        }
        ~mifareClassic_writeWorker() {}

        void Execute () {
            error = mifare_classic_write(tag, block, data);
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

        // Block to write
        MifareClassicBlockNumber block;
        MifareClassicBlock data;

        // Error ID or 0
        int error;

};
NAN_METHOD(Tag::mifareClassic_write) {
    Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
    Callback *callback = new Callback(info[2].As<v8::Function>());
    AsyncQueueWorker(new mifareClassic_writeWorker(callback, obj->tag, info[0]->Uint32Value(), reinterpret_cast<unsigned char*>(node::Buffer::Data(info[1]))));
}


class mifareClassic_incrementWorker : public AsyncWorker {
    public:
        mifareClassic_incrementWorker(Callback *callback, MifareTag tag, MifareClassicBlockNumber block, const uint32_t amount)
        : AsyncWorker(callback), tag(tag), block(block), amount(amount), error(0) {}
        ~mifareClassic_incrementWorker() {}

        void Execute () {
            error = mifare_classic_increment(tag, 2, 10);
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

        // Block to write
        MifareClassicBlockNumber block;
        uint32_t amount;

        // Error ID or 0
        int error;

};
NAN_METHOD(Tag::mifareClassic_increment) {
    Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
    Callback *callback = new Callback(info[2].As<v8::Function>());
    AsyncQueueWorker(new mifareClassic_incrementWorker(callback, obj->tag, info[0]->Uint32Value(), info[1]->Uint32Value()));
}



class mifareClassic_decrementWorker : public AsyncWorker {
    public:
        mifareClassic_decrementWorker(Callback *callback, MifareTag tag, MifareClassicBlockNumber block, const uint32_t amount)
        : AsyncWorker(callback), tag(tag), block(block), amount(amount), error(0) {}
        ~mifareClassic_decrementWorker() {}

        void Execute () {
            error = mifare_classic_decrement(tag, block, amount);
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

        // Block to write
        MifareClassicBlockNumber block;
        uint32_t amount;

        // Error ID or 0
        int error;

};
NAN_METHOD(Tag::mifareClassic_decrement) {
    Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
    Callback *callback = new Callback(info[2].As<v8::Function>());
    AsyncQueueWorker(new mifareClassic_decrementWorker(callback, obj->tag, info[0]->Uint32Value(), info[1]->Uint32Value()));
}


class mifareClassic_restoreWorker : public AsyncWorker {
    public:
        mifareClassic_restoreWorker(Callback *callback, MifareTag tag, MifareClassicBlockNumber block)
        : AsyncWorker(callback), tag(tag), block(block), error(0) {}
        ~mifareClassic_restoreWorker() {}

        void Execute () {
            error = mifare_classic_restore(tag, block);
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

        // Block to write
        MifareClassicBlockNumber block;

        // Error ID or 0
        int error;

};
NAN_METHOD(Tag::mifareClassic_restore) {
    Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
    Callback *callback = new Callback(info[1].As<v8::Function>());
    AsyncQueueWorker(new mifareClassic_restoreWorker(callback, obj->tag, info[0]->Uint32Value()));
}


class mifareClassic_transferWorker : public AsyncWorker {
    public:
        mifareClassic_transferWorker(Callback *callback, MifareTag tag, MifareClassicBlockNumber block)
        : AsyncWorker(callback), tag(tag), block(block), error(0) {}
        ~mifareClassic_transferWorker() {}

        void Execute () {
            error = mifare_classic_transfer(tag, block);
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

        // Block
        MifareClassicBlockNumber block;

        // Error ID or 0
        int error;

};
NAN_METHOD(Tag::mifareClassic_transfer) {
    Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
    Callback *callback = new Callback(info[1].As<v8::Function>());
    AsyncQueueWorker(new mifareClassic_transferWorker(callback, obj->tag, info[0]->Uint32Value()));
}
