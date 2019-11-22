#include <napi.h>
#include "hdr_histogram_wrap.h"
#include "hdr_histogram_iterator_wrap.h"

Napi::Object InitAll (Napi::Env env, Napi::Object exports) {
  HdrHistogramWrap::Init(env, exports);
  HdrHistogramIteratorWrap::Init(env, exports);
  return exports;
}

NODE_API_MODULE(Histogram, InitAll)
