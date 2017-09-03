//oh man this shit isn't anywhere near done yet so the code looks like shit

#include <stdio.h>
#include <stddef.h>
#include <node.h>
#include <cstring>
#include <nan.h>
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
using v8::Array;
using v8::Handle;

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

int check_args_for_array(Isolate* isolate, const FunctionCallbackInfo<Value>& args, int argc){
	if (args.Length() != argc){
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Incorrect number of arguments")));
		return false;
	}
	if (!args[0]->IsArray()){
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Arguments are of the wrong type")));
		return false;
	}

	if(argc == 2){
		if (!args[1]->IsNumber()){
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Arguments are of the wrong type")));
			return false;
		} else {
			if (args[1]->NumberValue() > Handle<Array>::Cast(args[0])->Length()){
				isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Sample size is longer than array length")));
			}
		}
	}

	return true;
}

Local<Array> fisher_yates(Local<Array> arr, uint32_t samplesize, Isolate* isolate){
	Local<Array> samples = Array::New(isolate, samplesize);

	for (uint32_t i = arr->Length() - 1; i>0; --i){
		int r = get_rand() % (i + 1);
		auto x = arr->Get(i);
		auto y = arr->Get(r);
		arr->Set(r, x);
		arr->Set(i, y);
	}

	if (samplesize < arr->Length()){
		for(uint32_t j = 0; j < samplesize; ++j){
			samples->Set(j, arr->Get(j));
		}
		return samples;
	} else return arr;
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

void Choice(const FunctionCallbackInfo<Value>& args){
	Isolate* isolate = args.GetIsolate();
	if (!check_args_for_array(isolate, args, 1)){
		return;
	}
	Local<Array> array = Local<Array>::Cast(args[0]);
	int32_t randomNumber = (int32_t) get_rand()%(array->Length());
	Local<Value> obj = array->Get(randomNumber);
	args.GetReturnValue().Set(obj);
}

void Choices(const FunctionCallbackInfo<Value>& args){
	Isolate* isolate = args.GetIsolate();
	if(!check_args_for_array(isolate, args, 2)){
		return;
	}
	Local<Array> array = Local<Array>::Cast(args[0]);
	if (args[0]->NumberValue() < 1 || args[0]->NumberValue() > array->Length()){
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Sample size is either 0 or too big")));
		return;
	}
	Local<Array> samples = fisher_yates(array, args[1]->NumberValue(), isolate);

	args.GetReturnValue().Set(samples);
}

void Shuffle(const FunctionCallbackInfo<Value>& args){
	Isolate* isolate = args.GetIsolate();
	if(!check_args_for_array(isolate, args, 1)){
		return;
	}
	Local<Array> array = Local<Array>::Cast(args[0]);
	Local<Array> shuffled = fisher_yates(array, array->Length(), isolate);

	args.GetReturnValue().Set(shuffled);
}

void Bytes(const FunctionCallbackInfo<Value>& args){
	Isolate* isolate = args.GetIsolate();

	if (args.Length() != 1){
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Incorrect number of arguments")));
		return;
	}
	if (!args[0]->IsNumber()){
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Arguments are of the wrong type")));
		return;
	}
	if (args[0]->NumberValue() < 1){
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Argument is too small")));
		return;
	}
	int size = args[0]->NumberValue();
	char* data = new char[size];

	int32_t r = get_rand();
	int counter = 0;
	for (int i = 0; i < args[0]->NumberValue(); i++){
		data[i] = (r >> (8*counter)) & 0xff;
		counter++;
		if (counter == 4){
			counter = 0;
			r = get_rand();
		}
	}
	Nan::MaybeLocal<v8::Object> buf = Nan::NewBuffer(data, size);
	args.GetReturnValue().Set(buf.ToLocalChecked());

}

void init(Local<Object> exports) {
	NODE_SET_METHOD(exports, "getInt", getInt);
	NODE_SET_METHOD(exports, "getFloat", getFloat);
	NODE_SET_METHOD(exports, "Initialize", Initialize);
	NODE_SET_METHOD(exports, "getIntInRange", getIntInRange);
	NODE_SET_METHOD(exports, "getFloatInRange", getFloatInRange);
	NODE_SET_METHOD(exports, "Choice", Choice);
	NODE_SET_METHOD(exports, "Choices", Choices);
	NODE_SET_METHOD(exports, "Shuffle", Shuffle);
	NODE_SET_METHOD(exports, "Bytes", Bytes);
}

NODE_MODULE(addon, init)
}
