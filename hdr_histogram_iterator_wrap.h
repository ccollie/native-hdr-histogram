//
// Created by Clayton Collie on 11/21/19.
//

#ifndef HDR_HISTOGRAM_ITERATOR_WRAP_H
#define HDR_HISTOGRAM_ITERATOR_WRAP_H
#include <napi.h>

extern "C" {
#include "hdr_histogram.h"
}

class HdrHistogramWrap;
class HdrHistogramIteratorWrap;

class HdrHistogramIteratorWrap : public Napi::ObjectWrap<HdrHistogramIteratorWrap> {
public:
    const static int TYPE_ALL       = 0;
    const static int TYPE_RECORDED  = 1;
    const static int TYPE_LOG       = 2;
    const static int TYPE_LINEAR    = 3;
    const static int TYPE_PERCENTILE = 4;

    static void Init(Napi::Env env, Napi::Object target);
    static Napi::Object NewInstance(Napi::Env env, Napi::Value histogram, int type, hdr_iter* iter);

    HdrHistogramIteratorWrap(const Napi::CallbackInfo& info);
    ~HdrHistogramIteratorWrap();

private:
    typedef Napi::Object(HdrHistogramIteratorWrap::*IteratorValueGetterFunction)(Napi::Env env);

    HdrHistogramWrap *histogram_;
    struct hdr_iter *iter_;
    int32_t type_ = TYPE_ALL;
    IteratorValueGetterFunction valueGetter_;

    static Napi::FunctionReference constructor;
    
    Napi::Value Next(const Napi::CallbackInfo& info);
    Napi::Value Iterator(const Napi::CallbackInfo& info);

    Napi::Object BaseValueGetter(Napi::Env env);
    Napi::Object AllValueGetter(Napi::Env env);
    Napi::Object PercentileValueGetter(Napi::Env env);
    Napi::Object RecordedValueGetter(Napi::Env env);
    Napi::Object LinearValueGetter(Napi::Env env);
    Napi::Object LogValueGetter(Napi::Env env);
};

#endif // HDR_HISTOGRAM_ITERATOR_WRAP_H
