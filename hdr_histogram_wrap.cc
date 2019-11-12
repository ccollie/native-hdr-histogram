#include <nan.h>
#include "hdr_histogram_wrap.h"

extern "C" {
#include "hdr_histogram.h"
#include "hdr_histogram_log.h"
}

Nan::Persistent<v8::Function> HdrHistogramWrap::constructor;
Nan::Persistent<v8::Value> HdrHistogramWrap::prototype;

NAN_MODULE_INIT(HdrHistogramWrap::Init) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("HdrHistogram").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "record", Record);
  Nan::SetPrototypeMethod(tpl, "recordCorrectedValue", RecordCorrectedValue);
  Nan::SetPrototypeMethod(tpl, "min", Min);
  Nan::SetPrototypeMethod(tpl, "max", Max);
  Nan::SetPrototypeMethod(tpl, "mean", Mean);
  Nan::SetPrototypeMethod(tpl, "stddev", Stddev);
  Nan::SetPrototypeMethod(tpl, "percentile", Percentile);
  Nan::SetPrototypeMethod(tpl, "encode", Encode);
  Nan::SetMethod(tpl, "decode", Decode);
  Nan::SetPrototypeMethod(tpl, "percentiles", Percentiles);
  Nan::SetPrototypeMethod(tpl, "add", Add);
  Nan::SetPrototypeMethod(tpl, "countAtValue", GetCountAtValue);
  Nan::SetPrototypeMethod(tpl, "valuesAreEquivalent", ValuesAreEquivalent);
  Nan::SetPrototypeMethod(tpl, "lowestEquivalentValue", LowestEquivalentValue);
  Nan::SetPrototypeMethod(tpl, "highestEquivalentValue", HighestEquivalentValue);
  Nan::SetPrototypeMethod(tpl, "nextNonEquivalentValue", NextNonEquivalentValue);
  Nan::SetPrototypeMethod(tpl, "reset", Reset);

  // Properties
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("lowestTrackableValue").ToLocalChecked(), HdrHistogramWrap::Getter);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("highestTrackableValue").ToLocalChecked(), HdrHistogramWrap::Getter);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("significantFigures").ToLocalChecked(), HdrHistogramWrap::Getter);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("totalCount").ToLocalChecked(), HdrHistogramWrap::Getter);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("memorySize").ToLocalChecked(), HdrHistogramWrap::Getter);

  constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
  Nan::Set(target, Nan::New("HdrHistogram").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

HdrHistogramWrap::~HdrHistogramWrap() {
  if (this->histogram) {
    delete this->histogram;
  }
}

NAN_METHOD(HdrHistogramWrap::New) {
  if (info.IsConstructCall()) {
    int64_t lowest = info[0]->IsUndefined() ? 1 : Nan::To<int64_t>(info[0]).FromJust();
    int64_t highest = info[1]->IsUndefined() ? 100 : Nan::To<int64_t>(info[1]).FromJust();
    int significant_figures = info[2]->IsUndefined() ? 3 : Nan::To<int>(info[2]).FromJust();

    if (lowest <= 0) {
      return Nan::ThrowError("The lowest trackable number must be greater than 0");
    }

    if (significant_figures < 1 || significant_figures > 5) {
      return Nan::ThrowError("The significant figures must be between 1 and 5 (inclusive)");
    }

    HdrHistogramWrap *obj = new HdrHistogramWrap();

    int init_result = hdr_init(
        lowest,
        highest,
        significant_figures,
        &obj->histogram);

    if (init_result != 0) {
      delete obj;
      return Nan::ThrowError("Unable to initialize the Histogram");
    }

    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    const int argc = 3;
    v8::Local<v8::Value> argv[argc] = {
      info[0],
      info[1],
      info[2]
    };
    v8::Local<v8::Function> cons = Nan::New(constructor);
    v8::MaybeLocal<v8::Object> wrap = Nan::NewInstance(cons, argc, argv);

    if (wrap.IsEmpty()) {
      return;
    }

    info.GetReturnValue().Set(wrap.ToLocalChecked());
  }

  if (prototype.IsEmpty()) {
    prototype.Reset(info.This()->GetPrototype());
  }
}

NAN_METHOD(HdrHistogramWrap::Record) {
  HdrHistogramWrap* obj = Nan::ObjectWrap::Unwrap<HdrHistogramWrap>(info.This());

  if (info[0]->IsUndefined()) {
    info.GetReturnValue().Set(false);
    return;
  }

  int64_t count = info[1]->IsUndefined() ? 1 : Nan::To<int64_t>(info[1]).FromJust();
  int64_t value = Nan::To<int64_t>(info[0]).FromJust();

  bool result = hdr_record_values(obj->histogram, value, count);
  info.GetReturnValue().Set(result);
}

NAN_METHOD(HdrHistogramWrap::RecordCorrectedValue) {
  HdrHistogramWrap* obj = Nan::ObjectWrap::Unwrap<HdrHistogramWrap>(info.This());

  if (info[0]->IsUndefined()) {
    info.GetReturnValue().Set(false);
    return;
  }

  if (info[1]->IsUndefined()) {
   return Nan::ThrowError("No interval specified");
  }

  int64_t value = Nan::To<int64_t>(info[0]).FromJust();
  int64_t expected_interval = Nan::To<int64_t>(info[1]).FromJust();
  int64_t count = info[2]->IsUndefined() ? 1 : Nan::To<int64_t>(info[2]).FromJust();

  bool result = hdr_record_corrected_values(obj->histogram, value, count, expected_interval);
  info.GetReturnValue().Set(result);
}

NAN_METHOD(HdrHistogramWrap::Min) {
  HdrHistogramWrap* obj = Nan::ObjectWrap::Unwrap<HdrHistogramWrap>(info.This());
  int64_t value = hdr_min(obj->histogram);
  info.GetReturnValue().Set((double) value);
}

NAN_METHOD(HdrHistogramWrap::Max) {
  HdrHistogramWrap* obj = Nan::ObjectWrap::Unwrap<HdrHistogramWrap>(info.This());
  int64_t value = hdr_max(obj->histogram);
  info.GetReturnValue().Set((double) value);
}

NAN_METHOD(HdrHistogramWrap::Mean) {
  HdrHistogramWrap* obj = Nan::ObjectWrap::Unwrap<HdrHistogramWrap>(info.This());
  double value = hdr_mean(obj->histogram);
  info.GetReturnValue().Set(value);
}

NAN_METHOD(HdrHistogramWrap::Stddev) {
  HdrHistogramWrap* obj = Nan::ObjectWrap::Unwrap<HdrHistogramWrap>(info.This());
  double value = hdr_stddev(obj->histogram);
  info.GetReturnValue().Set(value);
}

NAN_METHOD(HdrHistogramWrap::Percentile) {
  if (info[0]->IsUndefined()) {
    return Nan::ThrowError("No percentile specified");
  }

  double percentile = Nan::To<double>(info[0]).FromJust();

  if (percentile <= 0.0 || percentile > 100.0) {
    return Nan::ThrowError("percentile must be > 0 and <= 100");
  }

  HdrHistogramWrap* obj = Nan::ObjectWrap::Unwrap<HdrHistogramWrap>(info.This());
  double value = hdr_value_at_percentile(obj->histogram, percentile);
  info.GetReturnValue().Set(value);
}

NAN_METHOD(HdrHistogramWrap::Encode) {
  HdrHistogramWrap* obj = Nan::ObjectWrap::Unwrap<HdrHistogramWrap>(info.This());
  char *encoded;
  int result = hdr_log_encode(obj->histogram, &encoded);
  if (result != 0) {
    return Nan::ThrowError("failed to encode");
  }
  int len = strlen(encoded);
  Nan::MaybeLocal<v8::Object> buf = Nan::NewBuffer(encoded, len);
  info.GetReturnValue().Set(buf.ToLocalChecked());
}

NAN_METHOD(HdrHistogramWrap::Decode) {
  v8::Local<v8::Value> buf;
  if (info.Length() > 0 && info[0]->IsObject(), node::Buffer::HasInstance(info[0])) {
    buf = info[0];
  } else {
    return Nan::ThrowError("Missing Buffer");
  }
  char *encoded = node::Buffer::Data(buf);
  size_t len  = node::Buffer::Length(buf);
  const int argc = 0;
  v8::Local<v8::Function> cons = Nan::New(constructor);
  v8::Local<v8::Object> wrap = Nan::NewInstance(cons, argc, NULL).ToLocalChecked();

  HdrHistogramWrap* obj = Nan::ObjectWrap::Unwrap<HdrHistogramWrap>(wrap);

  hdr_log_decode(&obj->histogram, encoded, len);

  info.GetReturnValue().Set(wrap);
}

NAN_METHOD(HdrHistogramWrap::Percentiles) {
  HdrHistogramWrap* obj = Nan::ObjectWrap::Unwrap<HdrHistogramWrap>(info.This());
  v8::Local<v8::Array> result = Nan::New<v8::Array>();

  hdr_iter iter;
  hdr_iter_percentile_init(&iter, obj->histogram, 1);

  int count = 0;

  while(hdr_iter_next(&iter)) {
    v8::Local<v8::Object> percentile = Nan::New<v8::Object>();

    Nan::Set(
        percentile,
        Nan::New("percentile").ToLocalChecked(),
        Nan::New(iter.specifics.percentiles.percentile));

    Nan::Set(
        percentile,
        Nan::New("value").ToLocalChecked(),
        Nan::New((double) iter.value));

    Nan::Set(result, count++, percentile);
  }

  info.GetReturnValue().Set(result);
}

NAN_METHOD(HdrHistogramWrap::Add) {
  HdrHistogramWrap* obj = Nan::ObjectWrap::Unwrap<HdrHistogramWrap>(info.This());
  HdrHistogramWrap* from = CheckUnwrap(info[0]);

  if (!from) {
    return Nan::ThrowTypeError("HdrHistogram expected");
  }

  int64_t dropped;

  if (info[1]->IsUndefined()) {
    dropped = hdr_add(obj->histogram, from->histogram);
  } else {
    int64_t expected_interval = Nan::To<int64_t>(info[1]).FromJust();
    dropped = hdr_add_while_correcting_for_coordinated_omission(obj->histogram, from->histogram, expected_interval);
  }

  return info.GetReturnValue().Set((double) dropped); 
}

NAN_METHOD(HdrHistogramWrap::GetCountAtValue) {
  if (info[0]->IsUndefined()) {
    return Nan::ThrowError("No value specified");
  }

  int64_t value = Nan::To<int64_t>(info[0]).FromJust();

  HdrHistogramWrap* obj = Nan::ObjectWrap::Unwrap<HdrHistogramWrap>(info.This());

  int64_t count = hdr_count_at_value(obj->histogram, value);
  info.GetReturnValue().Set((double) count);
}

NAN_METHOD(HdrHistogramWrap::ValuesAreEquivalent) {

  if (info[0]->IsUndefined() || info[1]->IsUndefined()) {
    return Nan::ThrowError("Expect 2 values for comparison");
  }

  int64_t a = Nan::To<int64_t>(info[0]).FromJust();
  int64_t b = Nan::To<int64_t>(info[1]).FromJust();

  HdrHistogramWrap* obj = Nan::ObjectWrap::Unwrap<HdrHistogramWrap>(info.This());

  bool equivalent = hdr_values_are_equivalent(obj->histogram, a, b);
  info.GetReturnValue().Set(equivalent);
}

NAN_METHOD(HdrHistogramWrap::LowestEquivalentValue) {
  if (info[0]->IsUndefined()) {
    return Nan::ThrowError("No value specified");
  }

  int64_t value = Nan::To<int64_t>(info[0]).FromJust();

  HdrHistogramWrap* obj = Nan::ObjectWrap::Unwrap<HdrHistogramWrap>(info.This());

  int64_t equivalent = hdr_lowest_equivalent_value(obj->histogram, value);
  info.GetReturnValue().Set((double) equivalent);
}

NAN_METHOD(HdrHistogramWrap::HighestEquivalentValue) {
  if (info[0]->IsUndefined()) {
    return Nan::ThrowError("No value specified");
  }

  int64_t value = Nan::To<int64_t>(info[0]).FromJust();

  HdrHistogramWrap* obj = Nan::ObjectWrap::Unwrap<HdrHistogramWrap>(info.This());

  int64_t highest = hdr_next_non_equivalent_value(obj->histogram, value) - 1;
  info.GetReturnValue().Set((double) highest);
}

NAN_METHOD(HdrHistogramWrap::NextNonEquivalentValue) {
  if (info[0]->IsUndefined()) {
    return Nan::ThrowError("No value specified");
  }

  int64_t value = Nan::To<int64_t>(info[0]).FromJust();

  HdrHistogramWrap* obj = Nan::ObjectWrap::Unwrap<HdrHistogramWrap>(info.This());

  int64_t next = hdr_next_non_equivalent_value(obj->histogram, value);
  info.GetReturnValue().Set((double) next);
}

NAN_METHOD(HdrHistogramWrap::Reset) {
  HdrHistogramWrap* obj = Nan::ObjectWrap::Unwrap<HdrHistogramWrap>(info.This());
  hdr_reset(obj->histogram);
  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(HdrHistogramWrap::Getter) {
  HdrHistogramWrap* obj = Nan::ObjectWrap::Unwrap<HdrHistogramWrap>(info.Holder());
  std::string propertyName = std::string(*Nan::Utf8String(property));

  if (propertyName == "totalCount") {
    return info.GetReturnValue().Set((double) obj->histogram->total_count);
  } else if (propertyName == "lowestTrackableValue") {
    return info.GetReturnValue().Set((double) obj->histogram->lowest_trackable_value);
  } else if (propertyName == "highestTrackableValue") {
    return info.GetReturnValue().Set((double) obj->histogram->highest_trackable_value);
  } else if (propertyName == "significantFigures") {
    return info.GetReturnValue().Set((int) obj->histogram->significant_figures);
  } else if (propertyName == "memorySize") {
    return info.GetReturnValue().Set((double) hdr_get_memory_size(obj->histogram));
  }

  info.GetReturnValue().SetUndefined();
}
 
// Walks the prototype chain to make sure arg is a HdrHisogram or descendent 
HdrHistogramWrap *HdrHistogramWrap::CheckUnwrap(v8::Local<v8::Value> arg) {
  Nan::MaybeLocal<v8::Object> maybe = Nan::To<v8::Object>(arg);
  if (!maybe.IsEmpty()) {
    v8::Local<v8::Object> obj = maybe.ToLocalChecked();
    v8::Local<v8::Object> current = obj;

    do {
      v8::Local<v8::Value> proto = current->GetPrototype();
      if (proto.IsEmpty() || !proto->IsObject()) {
        break;
      }
      // arg is derived from HdrHistogram prototype
      if (proto == prototype) {
        return Nan::ObjectWrap::Unwrap<HdrHistogramWrap>(obj);
      }
      current = proto.As<v8::Object>();  
    } while (true);
  }
 
  return (HdrHistogramWrap *)NULL;
}
