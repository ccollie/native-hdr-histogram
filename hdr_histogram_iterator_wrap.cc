//
// Created by Clayton Collie on 11/21/19.
//
#include <napi.h>
#include <algorithm>
#include "hdr_histogram_wrap.h"
#include "hdr_histogram_iterator_wrap.h"

extern "C" {
#include "hdr_histogram.h"
}

Napi::FunctionReference HdrHistogramIteratorWrap::constructor;

void HdrHistogramIteratorWrap::Init(Napi::Env env, Napi::Object target) {
    Napi::HandleScope scope(env);

    Napi::Function ctor = DefineClass(env, "HdrHistogramIterator", {
        // establish es6 iterator protocol
        InstanceMethod(Napi::Symbol::WellKnown(env, "iterator"), &HdrHistogramIteratorWrap::Iterator, napi_default),
        InstanceMethod("next", &HdrHistogramIteratorWrap::Next, napi_default),
    });

    constructor = Napi::Persistent(ctor);
    constructor.SuppressDestruct();

    // this is an internal class, so we don't export
    // target.Set("HdrHistogramIterator", ctor);
}

HdrHistogramIteratorWrap::HdrHistogramIteratorWrap(const Napi::CallbackInfo& info): Napi::ObjectWrap<HdrHistogramIteratorWrap>(info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    HdrHistogramIteratorWrap::IteratorValueGetterFunction valueFunctions[] = {
        &HdrHistogramIteratorWrap::AllValueGetter, 
        &HdrHistogramIteratorWrap::RecordedValueGetter, 
        &HdrHistogramIteratorWrap::LogValueGetter,
        &HdrHistogramIteratorWrap::LinearValueGetter, 
        &HdrHistogramIteratorWrap::PercentileValueGetter
    };

    type_ = info[1].IsUndefined() ? TYPE_ALL : info[1].As<Napi::Number>().Int32Value();
    
    if (type_ < TYPE_ALL || type_ > TYPE_PERCENTILE) {
        throw Napi::TypeError::New(env, "Invalid iterator type");        
    }

    valueGetter_ = valueFunctions[type_];

    Napi::Object object = info[0].As<Napi::Object>();
    bool isHistogram = object.InstanceOf(HdrHistogramWrap::constructor.Value());
    if (!isHistogram) {
        throw Napi::Error::New(env, "Histogram expected");
    }

    Napi::Value arg = info[2];
    if (arg.Type() != napi_external) {
        throw Napi::TypeError::New(env, "An external argument was expected.");
    }

    Napi::External<hdr_iter> ext = arg.As<Napi::External<hdr_iter>>();
    hdr_iter* iter = (hdr_iter *)ext.Data();
    if (iter == nullptr) {
        throw Napi::Error::New(env, "Null iterator passed to constructor.");
    }

    HdrHistogramWrap *hist = HdrHistogramWrap::Unwrap(object);
    
    // create a heap based copy of iter (we manage memory)
    hdr_iter *iter_copy = new hdr_iter;
    memcpy (iter_copy, iter, sizeof(hdr_iter));

    this->iter_ = iter_copy;
    this->histogram_ = hist;
}

HdrHistogramIteratorWrap::~HdrHistogramIteratorWrap() {
    if (this->histogram_) {
        this->histogram_->Unref();
    }
    if (this->iter_) {
        delete this->iter_;
    }
}

Napi::Object HdrHistogramIteratorWrap::NewInstance(Napi::Env env, Napi::Value histogram, int type, hdr_iter* iter) {
    Napi::Number typeValue = Napi::Number::New(env, type);
    Napi::External<hdr_iter> iterator = Napi::External<hdr_iter>::New(env, iter);

    return constructor.New({histogram, typeValue, iterator});
}

// Iterator accessor`[Symbol.iterator]()`
Napi::Value HdrHistogramIteratorWrap::Iterator(const Napi::CallbackInfo& info) {
    return info.This();
}

Napi::Value HdrHistogramIteratorWrap::Next(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    bool has_value = hdr_iter_next(iter_);

    Napi::Object obj = Napi::Object::New(env);

    obj.Set("done", !has_value);

    if (has_value) {
        Napi::Object value = (this->*valueGetter_)(env);
        obj.Set("value", value);
    } else {
        obj.Set("value", env.Undefined());
    }

    return obj;
}

Napi::Object HdrHistogramIteratorWrap::BaseValueGetter(Napi::Env env) {
    Napi::Object obj = Napi::Object::New(env);

    obj.Set("value", (double) iter_->value);
    obj.Set("count", (double) iter_->count);
    obj.Set("valueIteratedFrom", (double) iter_->value_iterated_from);
    obj.Set("valueIteratedTo", (double) iter_->value_iterated_to);
    obj.Set("lowestEquivalentValue", (double) iter_->lowest_equivalent_value);
    obj.Set("highestEquivalentValue", (double) iter_->highest_equivalent_value);
    obj.Set("medianEquivalentValue", (double) iter_->median_equivalent_value);

    return obj;
}

Napi::Object HdrHistogramIteratorWrap::AllValueGetter(Napi::Env env) {
    Napi::Object obj = BaseValueGetter(env);

    obj.Set("countAddedThisIteration", (double) iter_->count);

    return obj;
}

Napi::Object HdrHistogramIteratorWrap::PercentileValueGetter(Napi::Env env) {
    Napi::Object obj = BaseValueGetter(env);

    // for some reason the iterator may return an out of range ( > 100 ) value
    double percentile = iter_->specifics.percentiles.percentile;
    if (percentile > 100.0) percentile = 100.0;

    obj.Set("cumulativeCount", (double) iter_->cumulative_count);
    obj.Set("percentile", percentile);

    return obj;
}

Napi::Object HdrHistogramIteratorWrap::RecordedValueGetter(Napi::Env env) {
    Napi::Object obj = BaseValueGetter(env);
    obj.Set("countAddedThisIteration", (double) iter_->specifics.recorded.count_added_in_this_iteration_step);
    return obj;
}

Napi::Object HdrHistogramIteratorWrap::LinearValueGetter(Napi::Env env) {
    Napi::Object obj = BaseValueGetter(env);

    obj.Set("countAddedThisIteration", (double) iter_->specifics.linear.count_added_in_this_iteration_step);
    
    return obj;
}

Napi::Object HdrHistogramIteratorWrap::LogValueGetter(Napi::Env env) {
    Napi::Object obj = BaseValueGetter(env);

    obj.Set("countAddedThisIteration", (double) iter_->specifics.log.count_added_in_this_iteration_step);
    
    return obj;
}
