#ifndef HDRHISTOGRAMWRAP_H
#define HDRHISTOGRAMWRAP_H

#include <napi.h>

extern "C" {
#include "hdr_histogram.h"
}

class HdrHistogramWrap : public Napi::ObjectWrap<HdrHistogramWrap> {
 public:
  static void Init(Napi::Env env, Napi::Object target);

  HdrHistogramWrap(const Napi::CallbackInfo& info);
  ~HdrHistogramWrap();

  Napi::Value Record(const Napi::CallbackInfo& info);
  Napi::Value Add(const Napi::CallbackInfo& info);
  Napi::Value Min(const Napi::CallbackInfo& info);
  Napi::Value Max(const Napi::CallbackInfo& info);
  Napi::Value Mean(const Napi::CallbackInfo& info);
  Napi::Value Stddev(const Napi::CallbackInfo& info);
  Napi::Value Percentile(const Napi::CallbackInfo& info);
  Napi::Value Encode(const Napi::CallbackInfo& info);
  Napi::Value Percentiles(const Napi::CallbackInfo& info);
  Napi::Value Reset(const Napi::CallbackInfo& info);
  Napi::Value ValuesAreEquivalent(const Napi::CallbackInfo& info);

  static Napi::Value Decode(const Napi::CallbackInfo& info);

  // Property accessors
  Napi::Value GetTotalCount(const Napi::CallbackInfo& info);
  Napi::Value GetHighestTrackableValue(const Napi::CallbackInfo& info);
  Napi::Value GetLowestTrackableValue(const Napi::CallbackInfo& info);
  Napi::Value GetSignificantFigures(const Napi::CallbackInfo& info);
  Napi::Value GetMemorySize(const Napi::CallbackInfo& info);

 private:
  static Napi::FunctionReference constructor;

  struct hdr_histogram *histogram;
};

#endif
