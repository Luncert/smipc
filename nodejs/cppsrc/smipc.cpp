#include <string>
#include <stdio.h>
#include <windows.h>
#include <napi.h>

using namespace Napi;

#define OP_SUCCEED 0
#define OP_FAILED -1
#define OPPOSITE_END_CLOSED -2

#define CHAN_R 0
#define CHAN_W 1

typedef void (*func_initLibrary)(int isTraceMode);
typedef void (*func_cleanLibrary)();
typedef int (*func_openChannel)(char *cid, int mode, int chanSz);
typedef int (*func_writeChannel)(char *cid, char *data, int len);
typedef int (*func_readChannel)(char *cid, char *buf, int n, char blocking);
typedef int (*func_printChannelStatus)(char *cid);
typedef int (*func_closeChannel)(char *cid);

func_initLibrary _initLibrary;
func_cleanLibrary _cleanLibrary;
func_openChannel _openChannel;
func_writeChannel _writeChannel;
func_readChannel _readChannel;
func_printChannelStatus _printChannelStatus;
func_closeChannel _closeChannel;

HMODULE module;

void Init(const CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsBoolean()) {
        Napi::TypeError::New(env, "Invalid parameter, (bool isTraceMode).")
            .ThrowAsJavaScriptException();
    }
    Boolean isTraceMode = info[0].As<Boolean>();
    _initLibrary(isTraceMode.Value() ? 1 : 0);
}

void Deinit(const CallbackInfo& info) {
    _cleanLibrary();
}

Boolean OpenChannel(const CallbackInfo& info) {
    Env env = info.Env();
    if (info.Length() < 3 || !info[0].IsString() || !info[1].IsNumber() || !info[2].IsNumber()) {
        TypeError::New(env, "Invalid parameter, (char *cid, int mode, int chanSz).")
            .ThrowAsJavaScriptException();
    }
    std::string cid = info[0].As<String>().Utf8Value();
    Number mode = info[1].As<Number>();
    Number chanSz = info[2].As<Number>();
    return Boolean::New(env,
        _openChannel((char*)cid.c_str(), mode.Int32Value(), chanSz.Int32Value()) == OP_SUCCEED);
}

Number ReadChannel(const CallbackInfo& info) {
    Env env = info.Env();
    if (info.Length() < 4 || !info[0].IsString() || !info[1].IsTypedArray()
        || !info[2].IsNumber() || !info[3].IsBoolean()) {
        Napi::TypeError::New(env, "Invalid parameter, (char *cid, char *buf, int n, bool blocking).")
            .ThrowAsJavaScriptException();
    }
    std::string cid = info[0].As<String>().Utf8Value();
    Uint8Array arr = info[1].As<Uint8Array>();
    Number n = info[2].As<Number>();
    Boolean blocking = info[3].As<Boolean>();

    char* data = reinterpret_cast<char *>(arr.ArrayBuffer().Data());
    int ret = _readChannel((char*)cid.c_str(), data, n.Int32Value(), blocking.Value() ? 1 : 0);
    return Number::New(env, ret);
}

Boolean WriteChannel(const CallbackInfo& info) {
    Env env = info.Env();
    if (info.Length() < 3 || !info[0].IsString() || !info[1].IsTypedArray() || !info[2].IsNumber()) {
        Napi::TypeError::New(env, "Invalid parameter, (char *cid, char *data, int n).")
            .ThrowAsJavaScriptException();
    }
    std::string cid = info[0].As<String>().Utf8Value();
    Uint8Array arr = info[1].As<Uint8Array>();
    Number n = info[2].As<Number>();

    char* data = reinterpret_cast<char *>(arr.ArrayBuffer().Data());
    return Boolean::New(env, _writeChannel((char*)cid.c_str(), data, n.Int32Value()) == OP_SUCCEED);
}

Boolean PrintChannelStatus(const CallbackInfo& info) {
    Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Invalid parameter, (char *cid).")
            .ThrowAsJavaScriptException();
    }
    // String的析构函数会自动释放内存，所以这里用cid来保存其引用
    std::string cid = info[0].As<String>().Utf8Value();
    return Boolean::New(env, _printChannelStatus((char*)cid.c_str()) == OP_SUCCEED);
}

Boolean CloseChannel(const CallbackInfo& info) {
    Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsString()) {
        TypeError::New(env, "Invalid parameter, (char *cid).")
            .ThrowAsJavaScriptException();
    }
    std::string cid = info[0].As<String>().Utf8Value();
    return Boolean::New(env, _closeChannel((char*)cid.c_str()) == OP_SUCCEED);
}

Object Initialize(Env env, Object exports) {
    // module = LoadLibrary("C:\\Workspace\\Project\\smipc\\core-dist\\libsmipc.dll");
    module = LoadLibrary("C:\\Workspace\\Project\\smipc\\core\\src\\cmake-build-debug\\libsmipc.dll");
    if (module == NULL) {
        printf("[ERROR] Failed to load libsmipc.dll\n");
        return exports;
    }
    _initLibrary = (func_initLibrary)GetProcAddress(module, "initLibrary");
    _cleanLibrary = (func_cleanLibrary)GetProcAddress(module, "cleanLibrary");
    _openChannel = (func_openChannel)GetProcAddress(module, "openChannel");
    _writeChannel = (func_writeChannel)GetProcAddress(module, "writeChannel");
    _readChannel = (func_readChannel)GetProcAddress(module, "readChannel");
    _printChannelStatus = (func_printChannelStatus)GetProcAddress(module, "printChannelStatus");
    _closeChannel = (func_closeChannel)GetProcAddress(module, "closeChannel");

    exports.Set("OP_SUCCEED", Number::New(env, OP_SUCCEED));
    exports.Set("OP_FAILED", Number::New(env, OP_FAILED));
    exports.Set("OPPOSITE_END_CLOSED", Number::New(env, OPPOSITE_END_CLOSED));

    exports.Set("CHAN_R", Number::New(env, CHAN_R));
    exports.Set("CHAN_W", Number::New(env, CHAN_W));

    exports.Set("init", Function::New(env, Init));
    exports.Set("deinit", Function::New(env, Deinit));
    exports.Set("openChannel", Function::New(env, OpenChannel));
    exports.Set("readChannel", Function::New(env, ReadChannel));
    exports.Set("writeChannel", Function::New(env, WriteChannel));
    exports.Set("printChannelStatus", Function::New(env, PrintChannelStatus));
    exports.Set("closeChannel", Function::New(env, CloseChannel));

    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Initialize)