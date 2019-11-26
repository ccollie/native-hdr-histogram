#include <napi.h>
#include "hdr_histogram_wrap.h"
#include "hdr_histogram_iterator_wrap.h"

extern "C" {
#include "hdr_histogram.h"
#include "hdr_histogram_log.h"
#include "hdr_histogram_ex.h"
}

Napi::FunctionReference HdrHistogramWrap::constructor;

void HdrHistogramWrap::Init(Napi::Env env, Napi::Object target) {
  Napi::HandleScope scope(env);

  Napi::Function ctor = DefineClass(env, "HdrHistogram", {
    InstanceMethod("record", &HdrHistogramWrap::Record),
    InstanceMethod("recordCorrectedValue", &HdrHistogramWrap::RecordCorrectedValue),
    InstanceMethod("add", &HdrHistogramWrap::Add),
    InstanceMethod("subtract", &HdrHistogramWrap::Subtract),
    InstanceMethod("copy", &HdrHistogramWrap::Copy),
    InstanceMethod("equals", &HdrHistogramWrap::Equals),
    InstanceMethod("min", &HdrHistogramWrap::Min),
    InstanceMethod("max", &HdrHistogramWrap::Max),
    InstanceMethod("mean", &HdrHistogramWrap::Mean),
    InstanceMethod("stddev", &HdrHistogramWrap::Stddev),
    InstanceMethod("percentile", &HdrHistogramWrap::Percentile),
    InstanceMethod("encode", &HdrHistogramWrap::Encode),
    InstanceMethod("percentiles", &HdrHistogramWrap::Percentiles),
    InstanceMethod("reset", &HdrHistogramWrap::Reset),
    InstanceMethod("countAtValue", &HdrHistogramWrap::GetCountAtValue),
    InstanceMethod("countBetweenValues", &HdrHistogramWrap::GetCountBetweenValues),
    InstanceMethod("valuesAreEquivalent", &HdrHistogramWrap::ValuesAreEquivalent),
    InstanceMethod("highestEquivalentValue", &HdrHistogramWrap::HighestEquivalentValue),
    InstanceMethod("lowestEquivalentValue", &HdrHistogramWrap::LowestEquivalentValue),
    InstanceMethod("nextNonEquivalentValue", &HdrHistogramWrap::NextNonEquivalentValue),

    StaticMethod("decode", &HdrHistogramWrap::Decode),

    // Iterators
    InstanceMethod("allValues", &HdrHistogramWrap::AllValues),
    InstanceMethod("recordedValues", &HdrHistogramWrap::RecordedValues),
    InstanceMethod("linearValues", &HdrHistogramWrap::LinearValues),
    InstanceMethod("logarithmicValues", &HdrHistogramWrap::LogarithmicValues),
    InstanceMethod("percentileValues", &HdrHistogramWrap::PercentileValues),

    InstanceAccessor("totalCount", &HdrHistogramWrap::GetTotalCount, nullptr),
    InstanceAccessor("highestTrackableValue", &HdrHistogramWrap::GetHighestTrackableValue, nullptr),
    InstanceAccessor("lowestTrackableValue", &HdrHistogramWrap::GetLowestTrackableValue, nullptr),
    InstanceAccessor("significantFigures", &HdrHistogramWrap::GetSignificantFigures, nullptr),
    InstanceAccessor("memorySize", &HdrHistogramWrap::GetMemorySize, nullptr)
  });

  constructor = Napi::Persistent(ctor);
  constructor.SuppressDestruct();

  target.Set("HdrHistogram", ctor);
}

HdrHistogramWrap::HdrHistogramWrap(const Napi::CallbackInfo& info) : Napi::ObjectWrap<HdrHistogramWrap>(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  int64_t lowest = info[0].IsUndefined() ? 1 : info[0].As<Napi::Number>().Int64Value();
  int64_t highest = info[1].IsUndefined() ? 100 : info[1].As<Napi::Number>().Int64Value();
  int significant_figures = info[2].IsUndefined() ? 3 : info[2].As<Napi::Number>().Int32Value();

  if (lowest <= 0) {
    throw Napi::Error::New(env, "The lowest trackable number must be greater than 0");
  }

  if (significant_figures < 1 || significant_figures > 5) {
    throw Napi::Error::New(env, "The significant figures must be between 1 and 5 (inclusive)");
  }

  int init_result = hdr_init(
      lowest,
      highest,
      significant_figures,
      &this->histogram);

  if (init_result != 0) {
    throw Napi::Error::New(env, "Unable to initialize the Histogram");
  }
}

HdrHistogramWrap::~HdrHistogramWrap() {
  if (this->histogram) {
    hdr_close(this->histogram);
  }
}

Napi::Value HdrHistogramWrap::Record(const Napi::CallbackInfo& info) {
  int64_t value;
  int64_t count;
  HdrHistogramWrap* obj = this;

  if (info[0].IsUndefined()) {
    return Napi::Boolean::New(info.Env(), false);
  }

  value = info[0].As<Napi::Number>().Int64Value();
  count = info[1].IsUndefined() ? 1 : info[1].As<Napi::Number>().Int64Value();

  bool result = hdr_record_values(obj->histogram, value, count);
  return Napi::Boolean::New(info.Env(), result);
}

Napi::Value HdrHistogramWrap::RecordCorrectedValue(const Napi::CallbackInfo& info) {
  HdrHistogramWrap* obj = this;
  Napi::Env env = info.Env();

  if (info[0].IsUndefined()) {
    return Napi::Boolean::New(env, false);
  }

  if (info[1].IsUndefined()) {
    Napi::Error::New(env, "Interval expected.").ThrowAsJavaScriptException();
    return Napi::Boolean::New(env, false);
  }

  int64_t value = info[0].As<Napi::Number>().Int64Value();
  int64_t expected_interval = info[1].As<Napi::Number>().Int64Value();
  int64_t count = info[2].IsUndefined() ? 1 : info[2].As<Napi::Number>().Int64Value();

  bool result = hdr_record_corrected_values(obj->histogram, value, count, expected_interval);
  return Napi::Boolean::New(env, result);
}

Napi::Value HdrHistogramWrap::Add(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  Napi::Object object = info[0].As<Napi::Object>();
  bool isHistogram = object.InstanceOf(HdrHistogramWrap::constructor.Value());
  if (!isHistogram) {
    Napi::TypeError::New(env, "Histogram expected.").ThrowAsJavaScriptException();
    return env.Null();
  }

  HdrHistogramWrap* obj = this;
  HdrHistogramWrap* from = HdrHistogramWrap::Unwrap(object);

  int64_t dropped;

  if (info[1].IsUndefined()) {
    dropped = hdr_add(obj->histogram, from->histogram);
  } else {
    int64_t expected_interval = info[1].As<Napi::Number>().Int64Value();
    dropped = hdr_add_while_correcting_for_coordinated_omission(obj->histogram, from->histogram, expected_interval);
  }

  return Napi::Number::New(env, (double)dropped);
}

Napi::Value HdrHistogramWrap::Subtract(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    Napi::Object object = info[0].As<Napi::Object>();
    bool isHistogram = object.InstanceOf(HdrHistogramWrap::constructor.Value());
    if (!isHistogram) {
        Napi::TypeError::New(env, "Histogram expected.").ThrowAsJavaScriptException();
        return env.Null();
    }

    HdrHistogramWrap* obj = this;
    HdrHistogramWrap* from = HdrHistogramWrap::Unwrap(object);

    int64_t dropped = hdr_subtract(obj->histogram, from->histogram);

    return Napi::Number::New(env, (double)dropped);
}

Napi::Value HdrHistogramWrap::Copy(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  Napi::Number lowest = Napi::Number::New(env, this->histogram->lowest_trackable_value);
  Napi::Number highest = Napi::Number::New(env, this->histogram->highest_trackable_value);
  Napi::Number significant_figures = Napi::Number::New(env, this->histogram->significant_figures);

  Napi::Object result = HdrHistogramWrap::constructor.New({ lowest, highest, significant_figures });
  HdrHistogramWrap* created = HdrHistogramWrap::Unwrap(result);

  hdr_add(created->histogram, this->histogram);
  
  return result;
}

Napi::Value HdrHistogramWrap::Equals(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  HdrHistogramWrap* obj = this;
  HdrHistogramWrap* other = (HdrHistogramWrap*)NULL;

  if (info[0].IsObject()) {
    Napi::Object object = info[0].As<Napi::Object>();
    if (object.InstanceOf(HdrHistogramWrap::constructor.Value())) {
      other = HdrHistogramWrap::Unwrap(object);
    } 
  }

  if (!other) {
    Napi::TypeError::New(env, "Histogram value expected.").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  bool isEqual = (obj == other) || hdr_equals(obj->histogram, other->histogram); 

  return Napi::Boolean::New(env, isEqual);
}

Napi::Value HdrHistogramWrap::Min(const Napi::CallbackInfo& info) {
  HdrHistogramWrap* obj = this;
  int64_t value = hdr_min(obj->histogram);
  return Napi::Number::New(info.Env(), (double) value);
}

Napi::Value HdrHistogramWrap::Max(const Napi::CallbackInfo& info) {
  HdrHistogramWrap* obj = this;
  int64_t value = hdr_max(obj->histogram);
  return Napi::Number::New(info.Env(), (double) value);
}

Napi::Value HdrHistogramWrap::Mean(const Napi::CallbackInfo& info) {
  HdrHistogramWrap* obj = this;
  double value = hdr_mean(obj->histogram);
  return Napi::Number::New(info.Env(), value);
}

Napi::Value HdrHistogramWrap::Stddev(const Napi::CallbackInfo& info) {
  HdrHistogramWrap* obj = this;
  double value = hdr_stddev(obj->histogram);
  return Napi::Number::New(info.Env(), value);
}

Napi::Value HdrHistogramWrap::Percentile(const Napi::CallbackInfo& info) {
  if (info[0].IsUndefined()) {
    Napi::Error::New(info.Env(), "No percentile specified").ThrowAsJavaScriptException();
  }

  double percentile = info[0].As<Napi::Number>().DoubleValue();

  if (percentile <= 0.0 || percentile > 100.0) {
    Napi::Error::New(info.Env(), "percentile must be > 0 and <= 100").ThrowAsJavaScriptException();
  }

  HdrHistogramWrap* obj = this;
  double value = hdr_value_at_percentile(obj->histogram, percentile);
  return Napi::Number::New(info.Env(), value);
}

Napi::Value HdrHistogramWrap::Encode(const Napi::CallbackInfo& info) {
  HdrHistogramWrap* obj = this;
  char *encoded;
  int result = hdr_log_encode(obj->histogram, &encoded);
  if (result != 0) {
    Napi::Error::New(info.Env(), "failed to encode").ThrowAsJavaScriptException();
  }
  int len = strlen(encoded);
  return Napi::Buffer<char>::New(info.Env(), encoded, len);
}

Napi::Value HdrHistogramWrap::Decode(const Napi::CallbackInfo& info) {
  Napi::Value buf;
  if (info.Length() > 0 && info[0].IsObject(), info[0].IsBuffer()) {
    buf = info[0];
  } else {
    throw Napi::Error::New(info.Env(), "Missing Buffer");
  }

  char *encoded = buf.As<Napi::Buffer<char>>().Data();
  size_t len  = buf.As<Napi::Buffer<char>>().Length();

  Napi::Object wrap = constructor.New(std::initializer_list<napi_value>{});
  HdrHistogramWrap* obj = HdrHistogramWrap::Unwrap(wrap);

  hdr_log_decode(&obj->histogram, encoded, len);

  return wrap;
}

Napi::Value HdrHistogramWrap::Percentiles(const Napi::CallbackInfo& info) {
  HdrHistogramWrap* obj = this;
  Napi::Env env = info.Env();
  Napi::Array result = Napi::Array::New(env);

  hdr_iter iter;
  hdr_iter_percentile_init(&iter, obj->histogram, 1);

  int count = 0;

  while(hdr_iter_next(&iter)) {
    Napi::Object percentile = Napi::Object::New(env);

    percentile.Set(Napi::String::New(env, "percentile"),
        Napi::Number::New(env, iter.specifics.percentiles.percentile));

    percentile.Set(Napi::String::New(env, "value"),
        Napi::Number::New(env, (double) iter.value));

    result.Set(count++, percentile);
  }

  return result;
}

Napi::Value HdrHistogramWrap::Reset(const Napi::CallbackInfo& info) {
  HdrHistogramWrap* obj = this;
  hdr_reset(obj->histogram);
  return info.This();
}

Napi::Value HdrHistogramWrap::GetCountAtValue(const Napi::CallbackInfo& info) {
  HdrHistogramWrap* obj = this;
  Napi::Env env = info.Env();

  if (info[0].IsUndefined()) {
    Napi::TypeError::New(env, "integer value expected.").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  int64_t value = info[0].As<Napi::Number>().Int64Value();
  int64_t count = hdr_count_at_value(obj->histogram, value);

  return Napi::Number::New(env, (double)count);
}

Napi::Value HdrHistogramWrap::GetCountBetweenValues(const Napi::CallbackInfo& info) {
  HdrHistogramWrap* obj = this;
  Napi::Env env = info.Env();

  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "number expected for lowValue.").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (!info[1].IsNumber()) {
    Napi::TypeError::New(env, "number expected for highValue.").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  int64_t low = info[0].As<Napi::Number>().Int64Value();
  int64_t high = info[1].As<Napi::Number>().Int64Value();

  int64_t count = hdr_count_between_values(obj->histogram, low, high);

  return Napi::Number::New(env, (double)count);
}

Napi::Value HdrHistogramWrap::ValuesAreEquivalent(const Napi::CallbackInfo& info) {
  HdrHistogramWrap* obj = this;
  Napi::Env env = info.Env();

  if (info[0].IsUndefined() || info[1].IsUndefined()) {
    Napi::Error::New(env, "Expected 2 values for comparison").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  int64_t a = info[0].As<Napi::Number>().Int64Value();
  int64_t b = info[1].As<Napi::Number>().Int64Value();

  bool equivalent = hdr_values_are_equivalent(obj->histogram, a, b);
  return Napi::Boolean::New(env, equivalent);
}

Napi::Value HdrHistogramWrap::LowestEquivalentValue(const Napi::CallbackInfo& info) {
  HdrHistogramWrap* obj = this;
  Napi::Env env = info.Env();
  
  if (info[0].IsUndefined()) {
    Napi::Error::New(env, "value expected.").ThrowAsJavaScriptException();
  }

  int64_t value = info[0].As<Napi::Number>().Int64Value();
  int64_t equivalent = hdr_lowest_equivalent_value(obj->histogram, value);

  return Napi::Number::New(env, (double)equivalent);
}

Napi::Value HdrHistogramWrap::HighestEquivalentValue(const Napi::CallbackInfo& info) {
  HdrHistogramWrap* obj = this;
  Napi::Env env = info.Env();

  if (info[0].IsUndefined()) {
    Napi::Error::New(env, "value expected.").ThrowAsJavaScriptException();
  }

  int64_t value = info[0].As<Napi::Number>().Int64Value();
  int64_t highest = hdr_next_non_equivalent_value(obj->histogram, value) - 1;
  
  return Napi::Number::New(env, (double)highest);
}

Napi::Value HdrHistogramWrap::NextNonEquivalentValue(const Napi::CallbackInfo& info) {
  HdrHistogramWrap* obj = this;
  Napi::Env env = info.Env();

  if (info[0].IsUndefined()) {
    Napi::Error::New(env, "value expected.").ThrowAsJavaScriptException();
  }

  int64_t value = info[0].As<Napi::Number>().Int64Value();
  int64_t next = hdr_next_non_equivalent_value(obj->histogram, value);
 
  return Napi::Number::New(env, (double)next);
}

/* #### ######## ######## ########     ###    ########  #######  ########   ######  */
/*  ##     ##    ##       ##     ##   ## ##      ##    ##     ## ##     ## ##    ## */
/*  ##     ##    ##       ##     ##  ##   ##     ##    ##     ## ##     ## ##       */
/*  ##     ##    ######   ########  ##     ##    ##    ##     ## ########   ######  */
/*  ##     ##    ##       ##   ##   #########    ##    ##     ## ##   ##         ## */
/*  ##     ##    ##       ##    ##  ##     ##    ##    ##     ## ##    ##  ##    ## */
/* ####    ##    ######## ##     ## ##     ##    ##     #######  ##     ##  ######  */

#define RETURN_ITERATOR(iter, type) \
  Napi::Object result = HdrHistogramIteratorWrap::NewInstance(info.Env(), info.This(), type, &iter); \
  this->Ref();  \
  return result;

Napi::Value HdrHistogramWrap::AllValues(const Napi::CallbackInfo& info) {
  hdr_iter iter;
  hdr_iter_init(&iter, this->histogram);

  RETURN_ITERATOR(iter, HdrHistogramIteratorWrap::TYPE_ALL)
}

Napi::Value HdrHistogramWrap::RecordedValues(const Napi::CallbackInfo& info) {
  hdr_iter iter;
  hdr_iter_recorded_init(&iter, this->histogram);

  RETURN_ITERATOR(iter, HdrHistogramIteratorWrap::TYPE_RECORDED)
}

Napi::Value HdrHistogramWrap::LinearValues(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "number value expected for valueUnitsPerBucket").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  int64_t value_units_per_bucket = info[0].As<Napi::Number>().Int64Value();

  hdr_iter iter;
  hdr_iter_linear_init(&iter, this->histogram, value_units_per_bucket);

  RETURN_ITERATOR(iter, HdrHistogramIteratorWrap::TYPE_LINEAR)
}

Napi::Value HdrHistogramWrap::LogarithmicValues(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "number value expected for valueUnitsPerBucket").ThrowAsJavaScriptException();
    return env.Undefined();
  }
  
  if (!info[1].IsNumber()) {
    Napi::TypeError::New(env, "number value expected for logBase").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  int64_t value_units_first_bucket = info[0].As<Napi::Number>().Int64Value();
  double log_base = info[1].As<Napi::Number>().DoubleValue();

  hdr_iter iter;
  hdr_iter_log_init(&iter, this->histogram, value_units_first_bucket, log_base);

  RETURN_ITERATOR(iter, HdrHistogramIteratorWrap::TYPE_LOG)
}

Napi::Value HdrHistogramWrap::PercentileValues(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  int32_t ticksPerHalfDistance = 1;
  if (!info[0].IsUndefined()) {
    if (!info[0].IsNumber()) {
      Napi::TypeError::New(env, "number value expected for ticksPerHalfDistance").ThrowAsJavaScriptException();
      return env.Undefined();
    }
    ticksPerHalfDistance = info[0].As<Napi::Number>().Int32Value();
  }

  hdr_iter iter;
  hdr_iter_percentile_init(&iter, this->histogram, ticksPerHalfDistance);

  RETURN_ITERATOR(iter, HdrHistogramIteratorWrap::TYPE_PERCENTILE)
}


Napi::Value HdrHistogramWrap::GetTotalCount(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), (double)this->histogram->total_count);
}

Napi::Value HdrHistogramWrap::GetHighestTrackableValue(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), (double)this->histogram->highest_trackable_value);
}

Napi::Value HdrHistogramWrap::GetLowestTrackableValue(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), (double)this->histogram->lowest_trackable_value);
}

Napi::Value HdrHistogramWrap::GetSignificantFigures(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), (double)this->histogram->significant_figures);
}

Napi::Value HdrHistogramWrap::GetMemorySize(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), (double)hdr_get_memory_size(this->histogram));
}