//oh man this shit isn't anywhere near done yet so the code looks like shit

#include <stdio.h>
#include <stddef.h>
#include <node.h>
#include <cstring>
#include "isaac.h"

int32_t buf_to_int32(uint8_t *buffer){
	int32_t val = 0;
	val |= buffer[0] << 24;
	val |= buffer[1] << 16;
	val |= buffer[2] << 8;
	val |= buffer[3];
	return val;
}

#ifdef _WIN32
#include <windows.h>

//TODO: windows get_random_int32()

#else
#include <sys/random.h>

int32_t get_random_int32(){
	uint8_t buff[4];
	getrandom(buff, 4, GRND_NONBLOCK);
	int32_t val = buf_to_int32(buff);
	return val;
}

#endif

namespace positivity{
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::Value;
using v8::Number;
using v8::Integer;
using v8::Exception;
using v8::String;

const int32_t MAX32 = 2147483647;

int check_args_for_range(Isolate* isolate, const FunctionCallbackInfo<Value>& args){
	if (args.Length() != 2){
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Incorrect number of arguments")));
		return false;
	}

	if (!args[0]->IsNumber() || !args[1]->IsNumber()) {
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Arguments are of the wrong type")));
		return false;
	}

	if (args[0]->NumberValue() > args[1]->NumberValue()){
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Min is greater than max")));
		return false;
	}

	return true;
}

void Initialize(const FunctionCallbackInfo<Value>& args){
	for (int i = 0; i < 256; ++i){
		randrsl[i] = get_random_int32();
	}
	randinit(get_random_int32());
	get_rand();
}

void getInt(const FunctionCallbackInfo<Value>& args){
	Isolate* isolate = args.GetIsolate();
	args.GetReturnValue().Set(Integer::New(isolate, get_rand()));
}

void getFloat(const FunctionCallbackInfo<Value>& args){
	Isolate* isolate = args.GetIsolate();
	double randomNumber = ((double)(int32_t)get_rand() / MAX32);
	args.GetReturnValue().Set(Number::New(isolate, randomNumber));
}

void getIntInRange(const FunctionCallbackInfo<Value>& args){
	Isolate* isolate = args.GetIsolate();
	if (!check_args_for_range(isolate, args)){
		return;
	}

	double val1 = args[0]->NumberValue(), val2 = args[1]->NumberValue();

	(val1 > 0) ? val1 += 0.5 : val1 -= 0.5;
	(val2 > 0) ? val2 += 0.5 : val2 -= 0.5;

	int32_t min = (int32_t)val1;
	int32_t max = (int32_t)val2;

	int32_t randomNumber = (int32_t) get_rand()%(max-min + 1) + min;
	args.GetReturnValue().Set(Integer::New(isolate, randomNumber));
}

void getFloatInRange(const FunctionCallbackInfo<Value>& args){
	Isolate* isolate = args.GetIsolate();
	if (!check_args_for_range(isolate, args)){
		return;
	}

	double min = args[0]->NumberValue(), max = args[1]->NumberValue();

	double f = (double) (int32_t) get_rand() / MAX32;
  double randomNumber = min + f * (max - min);

	args.GetReturnValue().Set(Number::New(isolate, randomNumber));
}

void init(Local<Object> exports) {
	NODE_SET_METHOD(exports, "getInt", getInt);
	NODE_SET_METHOD(exports, "getFloat", getFloat);
	NODE_SET_METHOD(exports, "Initialize", Initialize);
	NODE_SET_METHOD(exports, "getIntInRange", getIntInRange);
	NODE_SET_METHOD(exports, "getFloatInRange", getFloatInRange);
}

NODE_MODULE(addon, init)
}
