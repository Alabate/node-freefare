#include "tag.h"

using namespace Nan;


class mifareDesfire_connectWorker : public AsyncWorker {
    public:
        mifareDesfire_connectWorker(Callback *callback, MifareTag tag)
        : AsyncWorker(callback), tag(tag), error(0) {}
        ~mifareDesfire_connectWorker() {}

        void Execute () {
            error = mifare_desfire_connect(tag);
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
NAN_METHOD(Tag::mifareDesfire_connect) {
    Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
    Callback *callback = new Callback(info[0].As<v8::Function>());
    AsyncQueueWorker(new mifareDesfire_connectWorker(callback, obj->tag));
}


class mifareDesfire_disconnectWorker : public AsyncWorker {
    public:
        mifareDesfire_disconnectWorker(Callback *callback, MifareTag tag)
        : AsyncWorker(callback), tag(tag), error(0) {}
        ~mifareDesfire_disconnectWorker() {}

        void Execute () {
            error = mifare_desfire_disconnect(tag);
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
NAN_METHOD(Tag::mifareDesfire_disconnect) {
    Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
    Callback *callback = new Callback(info[0].As<v8::Function>());
    AsyncQueueWorker(new mifareDesfire_disconnectWorker(callback, obj->tag));
}


class mifareDesfire_authenticateWorker : public AsyncWorker {
    public:
        mifareDesfire_authenticateWorker(Callback *callback, MifareTag tag, const uint8_t key_no, MifareDESFireKey key)
        : AsyncWorker(callback), tag(tag), key_no(key_no), key(key), error(0) {}
        ~mifareDesfire_authenticateWorker() {}

        void Execute () {
            error = mifare_desfire_authenticate(tag, key_no, key);
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

        // Key
    	uint8_t key_no;
    	MifareDESFireKey key;

        // Error ID or 0
        int error;

};
NAN_METHOD(Tag::mifareDesfire_authenticate_des) {
    Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());

    MifareDESFireKey key = mifare_desfire_des_key_new(reinterpret_cast<uint8_t*>(node::Buffer::Data(info[1])));

    AsyncQueueWorker(new mifareDesfire_authenticateWorker(
        new Callback(info[2].As<v8::Function>()),
        obj->tag,
        info[0]->Uint32Value(),
        key
    ));

    mifare_desfire_key_free(key);
}
NAN_METHOD(Tag::mifareDesfire_authenticate_3des) {
    Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());

    MifareDESFireKey key = mifare_desfire_3des_key_new(reinterpret_cast<uint8_t*>(node::Buffer::Data(info[1])));

    AsyncQueueWorker(new mifareDesfire_authenticateWorker(
        new Callback(info[2].As<v8::Function>()),
        obj->tag,
        info[0]->Uint32Value(),
        key
    ));

    mifare_desfire_key_free(key);
}

class mifareDesfire_getApplicationIdsWorker : public AsyncWorker {
    public:
        mifareDesfire_getApplicationIdsWorker(Callback *callback, MifareTag tag)
        : AsyncWorker(callback), tag(tag), error(0) {}

        ~mifareDesfire_getApplicationIdsWorker() {}

        void Execute () {
            error = mifare_desfire_get_application_ids(tag, &aids, &count);
        }

        void HandleOKCallback () {
            Nan::HandleScope scope;

            uint32_t aid;
            v8::Local<v8::Array> results = New<v8::Array>(count);
            for (uint32_t d = 0; d < count; d++) {
                aid = 0;
                memcpy(&aid, reinterpret_cast<uint8_t*>(aids[d]), 3);
                aid = htole32(aid);
                Nan::Set(results, New<v8::Number>(d), New<v8::Number>(aid));
            }
            mifare_desfire_free_application_ids(aids);

            v8::Local<v8::Value> argv[] = {
                New<v8::Number>(error),
                results,
            };

            callback->Call(2, argv);
        }
    private:
        // Our current tag
    	MifareTag tag;

        MifareDESFireAID* aids;
        size_t count;

        // Error ID or 0
        int error;

};
NAN_METHOD(Tag::mifareDesfire_getApplicationIds) {
    Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
    Callback *callback = new Callback(info[0].As<v8::Function>());
    AsyncQueueWorker(new mifareDesfire_getApplicationIdsWorker(callback, obj->tag));
}



class mifareDesfire_selectApplicationWorker : public AsyncWorker {
    public:
        mifareDesfire_selectApplicationWorker(Callback *callback, MifareTag tag, uint32_t aid)
        : AsyncWorker(callback), tag(tag), aid(aid), error(0) {}

        ~mifareDesfire_selectApplicationWorker() {}

        void Execute () {
            error = mifare_desfire_select_application(tag, mifare_desfire_aid_new(aid));
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

        uint32_t aid;

        // Error ID or 0
        int error;

};
NAN_METHOD(Tag::mifareDesfire_selectApplication) {
    Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
    Callback *callback = new Callback(info[1].As<v8::Function>());
    AsyncQueueWorker(new mifareDesfire_selectApplicationWorker(callback, obj->tag, info[0]->Uint32Value()));
}




class mifareDesfire_getFileIdsWorker : public AsyncWorker {
    public:
        mifareDesfire_getFileIdsWorker(Callback *callback, MifareTag tag)
        : AsyncWorker(callback), tag(tag), error(0) {}

        ~mifareDesfire_getFileIdsWorker() {}

        void Execute () {
            error = mifare_desfire_get_file_ids(tag, &files, &count);
        }

        void HandleOKCallback () {
            Nan::HandleScope scope;

            v8::Local<v8::Array> results = New<v8::Array>(count);
            for (uint32_t d = 0; d < count; d++) {
                Nan::Set(results, New<v8::Number>(d), New<v8::Number>(files[d]));
            }

            v8::Local<v8::Value> argv[] = {
                New<v8::Number>(error),
                results,
            };

            callback->Call(2, argv);
        }
    private:
        // Our current tag
    	MifareTag tag;

        uint8_t* files;
        size_t count;

        // Error ID or 0
        int error;

};
NAN_METHOD(Tag::mifareDesfire_getFileIds) {
    Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
    Callback *callback = new Callback(info[0].As<v8::Function>());
    AsyncQueueWorker(new mifareDesfire_getFileIdsWorker(callback, obj->tag));
}


class mifareDesfire_readWorker : public AsyncWorker {
    public:
        mifareDesfire_readWorker(Callback *callback, MifareTag tag, uint8_t file, off_t offset, size_t length)
        : AsyncWorker(callback), tag(tag), file(file), offset(offset), length(length), error(0) {}
        ~mifareDesfire_readWorker() {}

        void Execute () {
            data = (uint8_t*) malloc((length+1)*sizeof(uint8_t));
            error = mifare_desfire_read_data(tag, file, offset, length, data);
        }

        void HandleOKCallback () {
            Nan::HandleScope scope;

            v8::Local<v8::Value> buf = Null();
            if(error > 0) {
                buf =  Nan::CopyBuffer(reinterpret_cast<char*>(data), length).ToLocalChecked();
                error = 0;
            }
            free(data);

            v8::Local<v8::Value> argv[] = {
                New<v8::Number>(error),
                buf
            };

            callback->Call(2, argv);
        }
    private:

        // Our current tag
    	MifareTag tag;

        uint8_t file;
        off_t offset;
        size_t length;
        uint8_t* data;

        // Error ID or 0
        int error;

};
NAN_METHOD(Tag::mifareDesfire_read) {
    Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
    Callback *callback = new Callback(info[3].As<v8::Function>());
    AsyncQueueWorker(new mifareDesfire_readWorker(callback, obj->tag, info[0]->Uint32Value(), info[1]->Uint32Value(), info[2]->Uint32Value()));
}


class mifareDesfire_writeWorker : public AsyncWorker {
    public:
        mifareDesfire_writeWorker(Callback *callback, MifareTag tag, uint8_t file, off_t offset, size_t length, uint8_t *data)
        : AsyncWorker(callback), tag(tag), file(file), offset(offset), length(length), error(0) {
            memcpy(this->data, data, length);
        }
        ~mifareDesfire_writeWorker() {}

        void Execute () {
            error = mifare_desfire_write_data(tag, file, offset, length, data);
        }

        void HandleOKCallback () {
            Nan::HandleScope scope;

            int count = 0;
            if(error > 0) {
                count = error;
                error = 0;
            }

            v8::Local<v8::Value> argv[] = {
                New<v8::Number>(error),
                New<v8::Number>(count),
            };

            callback->Call(1, argv);
        }
    private:

        // Our current tag
    	MifareTag tag;

        uint8_t file;
        off_t offset;
        size_t length;
        uint8_t* data;

        // Error ID or 0
        int error;

};
NAN_METHOD(Tag::mifareDesfire_write) {
    Tag* obj = ObjectWrap::Unwrap<Tag>(info.This());
    Callback *callback = new Callback(info[4].As<v8::Function>());
    AsyncQueueWorker(new mifareDesfire_writeWorker(callback, obj->tag, info[0]->Uint32Value(), info[1]->Uint32Value(), info[2]->Uint32Value(), reinterpret_cast<unsigned char*>(node::Buffer::Data(info[3]))));
}
