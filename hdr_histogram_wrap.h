#ifndef HDRHISTOGRAMWRAP_H
#define HDRHISTOGRAMWRAP_H

#include <nan.h>

extern "C" {
#include "hdr_histogram.h"
}

class HdrHistogramWrap : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

 private:
  ~HdrHistogramWrap();

  static void New(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void Record(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void RecordCorrectedValue(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void Min(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void Max(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void Mean(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void Stddev(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void Percentile(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void Encode(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void Decode(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void Percentiles(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void Add(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void GetCountAtValue(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void ValuesAreEquivalent(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void LowestEquivalentValue(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void HighestEquivalentValue(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void NextNonEquivalentValue(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void Reset(const Nan::FunctionCallbackInfo<v8::Value>& info);
  
  static NAN_GETTER(Getter);

  static HdrHistogramWrap *CheckUnwrap(v8::Local<v8::Value> obj);

  static Nan::Persistent<v8::Function> constructor;
  static Nan::Persistent<v8::Value> prototype;

  struct hdr_histogram *histogram;
};

#endif
