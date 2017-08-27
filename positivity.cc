//oh man this shit isn't anywhere near done yet so the code looks like shit

#include <stdio.h>
#include <stddef.h>
#include <node.h>
#include "ISAAC.cc"

uint32_t buf_to_uint32(uint8_t *buffer){
	uint32_t val = 0;
	val |= buffer[0] << 24;
	val |= buffer[1] << 16;
	val |= buffer[2] << 8;
	val |= buffer[3];
	return val;
}

#ifdef _WIN32
#include <windows.h>

//TODO: windows get_random_uint32()

#else
#include <sys/random.h>

uint32_t get_random_uint32(){
	uint8_t buff[4];
	getrandom(buff, 4, GRND_NONBLOCK);
	uint32_t val = buf_to_uint32(buff);
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

void Initialize(const FunctionCallbackInfo<Value>& args){
	for (int i = 0; i < 256; ++i){
		randrsl[i] = get_random_uint32();
	}
	randinit(get_random_uint32());
	get_rand();
}

void Random(const FunctionCallbackInfo<Value>& args){
	Isolate* isolate = args.GetIsolate();
	args.GetReturnValue().Set(Integer::NewFromUnsigned(isolate, get_rand()));
}

void Randint(const FunctionCallbackInfo<Value>& args){
	Isolate* isolate = args.GetIsolate();
	if (args.Length() != 2){
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Incorrect number of arguments")));
		return;
	}

	if (!args[0]->IsNumber() || !args[1]->IsNumber()) {
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Arguments are of the wrong type")));
		return;
	}

	if (args[0]->NumberValue() > args[1]->NumberValue()){
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Min is greater than max")));
		return;
	}

	double val1 = args[0]->NumberValue(), val2 = args[1]->NumberValue();

	(val1 > 0) ? val1 += 0.5 : val1 -= 0.5;
	(val2 > 0) ? val2 += 0.5 : val2 -= 0.5;

	uint32_t min = (uint32_t)val1;
	uint32_t max = (uint32_t)val2;

	uint32_t randomNumber = get_rand()%(max-min + 1) + min;
	args.GetReturnValue().Set(Integer::NewFromUnsigned(isolate, randomNumber));
}

void init(Local<Object> exports) {
	NODE_SET_METHOD(exports, "Random", Random);
	NODE_SET_METHOD(exports, "Initialize", Initialize);
	NODE_SET_METHOD(exports, "Randint", Randint);
}

NODE_MODULE(addon, init)
}
