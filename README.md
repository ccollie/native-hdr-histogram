# native-hdr-histogram

node.js bindings for [hdr histogram][hdr] [C implementation][cimpl](version 0.9.10).

[![Build status](https://ci.appveyor.com/api/projects/status/tgu1of6olrg64so4?svg=true)](https://ci.appveyor.com/project/mcollina/native-hdr-histogram)
[![Build Status](https://travis-ci.org/mcollina/native-hdr-histogram.svg?branch=master)](https://travis-ci.org/mcollina/native-hdr-histogram)

> HDR Histogram is designed for recoding histograms of value measurements
in latency and performance sensitive applications. Measurements show
value recording times as low as 3-6 nanoseconds on modern (circa 2014)
Intel CPUs. A Histogram's memory footprint is constant, with no
allocation operations involved in recording data values or in iterating through them.
- from [hdr histogram][hdr] website

This library is blazingly fast, and you can use it to record
histograms with no overhead. Linux, Mac OS X and Windows are all
supported.

  * <a href="#install">Installation</a>
  * <a href="#example">Example</a>
  * <a href="#api">API</a>
  * <a href="#licence">Licence &amp; copyright</a>

## Install

```
npm i native-hdr-histogram --save
```

If you see any errors, you might need to configure your system to compile native addons:
follow the instructions at [node-gyp][node-gyp].

## Example

```js
'use strict'

const Histogram = require('native-hdr-histogram')
const max = 1000000
const key = 'record*' + max
const histogram = new Histogram(1, 100)

console.time(key)
for (let i = 0; i < max; i++) {
  histogram.record(Math.floor((Math.random() * 42 + 1)))
}
console.timeEnd(key)

console.log('80 percentile is', histogram.percentile(80))
console.log('99 percentile is', histogram.percentile(99))

console.log(histogram.percentiles())
```

## API

  * <a href="#histogram"><code>Histogram</code></a>
  * <a href="#record"><code>histogram#<b>record()</b></code></a>
  * <a href="#recordCorrectedValue"><code>histogram#<b>recordCorrectedValue()</b></code></a>
  * <a href="#add"><code>histogram#<b>add()</b></code></a>
  * <a href="#equals"><code>histogram#<b>equals()</b></code></a>
  * <a href="#min"><code>histogram#<b>min()</b></code></a>
  * <a href="#max"><code>histogram#<b>max()</b></code></a>
  * <a href="#mean"><code>histogram#<b>mean()</b></code></a>
  * <a href="#stddev"><code>histogram#<b>stddev()</b></code></a>
  * <a href="#percentile"><code>histogram#<b>percentile()</b></code></a>
  * <a href="#percentiles"><code>histogram#<b>percentiles()</b></code></a>
  * <a href="#encode"><code>histogram#<b>encode()</b></code></a>
  * <a href="#decode"><code>histogram#<b>decode()</b></code></a>
  * <a href="#reset"><code>histogram#<b>reset()</b></code></a>
  * <a href="#countAtValue"><code>histogram#<b>countAtValue()</b></code></a>
  * <a href="#lowestEquivalentValue"><code>histogram#<b>lowestEquivalentValue()</b></code></a>
  * <a href="#highestEquivalentValue"><code>histogram#<b>highestEquivalentValue()</b></code></a>
  * <a href="#nextNonEquivalentValue"><code>histogram#<b>nextNonEquivalentValue()</b></code></a>
  * <a href="#valuesAreEquivalent"><code>histogram#<b>valuesAreEquivalent()</b></code></a>
    
  #### Properties
  * <a href="#lowestTrackableValue"><code>histogram#lowestTrackableValue</code></a>
  * <a href="#highestTrackableValue"><code>histogram#highestTrackableValue</code></a>
  * <a href="#significantFigures"><code>histogram#significantFigures</code></a>
  * <a href="#totalCount"><code>histogram#totalCount</code></a>
  * <a href="#memorySize"><code>histogram#memorySize</code></a>

  ### Iteration
  The module provides multiple means of <a href="#iteration">iterating</a> through the data
  * <a href="#percentileValues"><code>histogram#percentileValues()</code></a>
  * <a href="#linearValues"><code>histogram#linearValues()</code></a>
  * <a href="#logarithmicValues"><code>histogram#logarithmicValues()</code></a>
  * <a href="#recordedValues"><code>histogram#recordedValues()</code></a>
  * <a href="#allValues"><code>histogram#allValues()</code></a>
  
  
-------------------------------------------------------
<a name="histogram"></a>

### new Histogram(lowest, max, figures)

Create a new histogram with:

* `lowest`: is the lowest possible number that can be recorded (default
  1).
* `max`: is the maximum number that can be recorded (default 100).
* `figures`: the number of figures in a decimal number that will be
  maintained, must be between 1 and 5 (inclusive) (default 3).

-------------------------------------------------------
<a name="record"></a>

### histogram.record(value[, count = 1])

Record `value` in the histogram with a count of `count`. Returns `true` if the recording was
successful, `false` otherwise.
-------------------------------------------------------
<a name="recordCorrectedValue"></a>

### histogram.recordCorrectedValue(value, expectedInterval, count = 1)

Record `value` in the histogram with a count of `count` and backfill based on a `expectedInterval`.
This is specifically used for recording latency.  If `value` is larger than the `expectedInterval` 
then the latency recording system has experienced co-ordinated omission.  This method fills in the
values that would have occurred had the client providing the load not been blocked.

Returns `true` if the recording was successful, `false` otherwise.

-------------------------------------------------------
<a name="add"></a>

### histogram.add(other[, expectedIntervalBetweenValueSamples])

Adds values from `other` to 'this' histogram.  

If `expectedIntervalBetweenValueSamples` is specified, values are 
backfilled with values that would have occurred had the client providing the load 
not been blocked. The values added will include an auto-generated additional series of
decreasingly-smaller (down to the `expectedIntervalBetweenValueSamples`) value records for each count found
in the current histogram that is larger than the `expectedIntervalBetweenValueSamples`.
     
Returns the number of values dropped while copying. Values will be dropped
if they around outside of `histogram.lowestTrackableValue` and  `histogram.highestTrackableValue`. 

-------------------------------------------------------
<a name="equals"></a>

### histogram.equals(other)

Determine if this histogram is equivalent to another.

-------------------------------------------------------
<a name="min"></a>

### histogram.min()

Return the minimum value recorded in the histogram.

-------------------------------------------------------
<a name="max"></a>

### histogram.max()

Return the maximum value recorded in the histogram.

-------------------------------------------------------
<a name="mean"></a>

### histogram.mean()

Return the mean of the histogram.

-------------------------------------------------------
<a name="stddev"></a>

### histogram.stddev()

Return the standard deviation of the histogram.

-------------------------------------------------------
<a name="percentile"></a>

### histogram.percentile(percentile)

Returns the value at the given percentile. `percentile` must be >
0 and <= 100, otherwise it will throw.

-------------------------------------------------------
<a name="percentiles"></a>

### histogram.percentiles()

Returns all the percentiles.

Sample output:

```js
[ { percentile: 0, value: 1 },
  { percentile: 50, value: 22 },
  { percentile: 75, value: 32 },
  { percentile: 87.5, value: 37 },
  { percentile: 93.75, value: 40 },
  { percentile: 96.875, value: 41 },
  { percentile: 98.4375, value: 42 },
  { percentile: 100, value: 42 } ]
```

-------------------------------------------------------
<a name="encode"></a>

### histogram.encode()

Returns a `Buffer` containing a serialized version of the histogram

-------------------------------------------------------
<a name="decode"></a>

### histogram.decode(buf)

Reads a `Buffer` and deserialize an histogram.

-------------------------------------------------------
<a name="reset"></a>

### histogram.reset()

Resets the histogram so it can be reused.

-------------------------------------------------------
<a name="countAtValue"></a>

### histogram.countAtValue(value)

Get the count of recorded values at a specific value (to within the histogram resolution at the value level).

-------------------------------------------------------
<a name="lowestEquivalentValue"></a>

### histogram.lowestEquivalentValue(value)

Get the lowest value that is equivalent to the given value within the 
histogram's resolution, where "equivalent" means that value samples 
recorded for any two equivalent values are counted in a common total count.

------------------------------------------------------
<a name="highestEquivalentValue"></a>

### histogram.highestEquivalentValue(value)

Get the highest value that is equivalent to the given value within the 
histogram's resolution, where "equivalent" means that value samples 
recorded for any two equivalent values are counted in a common total count.

------------------------------------------------------
<a name="nextNonEquivalentValue"></a>

### histogram.nextNonEquivalentValue(value)

Get the next value that is not equivalent to the given value within the histogram's resolution.

------------------------------------------------------
<a name="valuesAreEquivalent"></a>

### histogram.valuesAreEquivalent(value1, value2)

Determine if two values are equivalent within the histogram's resolution
where "equivalent" means that value samples recorded for any two
equivalent values are counted in a common total count.
-------------------------------------------------------
<a name="properties"></a>
## Properties

<a name="lowestTrackableValue"></a>
### histogram.lowestTrackableValue

Get the configured lowestTrackableValue

-------------------------------------------------------
<a name="highestTrackableValue"></a>
### histogram.highestTrackableValue

Get the configured highestTrackableValue

-------------------------------------------------------
<a name="significantFigures"></a>
### histogram.significantFigures

Get the configured number of significant value digits

-------------------------------------------------------
<a name="totalCount"></a>
### histogram.totalCount

Gets the total number of recorded values.

-------------------------------------------------------
<a name="memorySize"></a>
### histogram.memorySize

Get the memory size of the Histogram.

-------------------------------------------------------
##Iteration
<a name="iteration"></a>

Histograms supports multiple convenient forms of iterating through the
histogram data set, including linear, logarithmic, and percentile iteration
mechanisms, as well as means for iterating through each recorded value or
each possible value level. The iteration mechanisms are accessible through
the through the following Histogram methods:

<a name="percentileValues"></a>
 - `histogram.percentileValues(ticksPerHalfDistance)`: <br/>  Used for iterating through histogram values according to percentile levels.
 The iteration is performed in steps that start at 0% and reduce their distance to 100% according to the
 <i>ticksPerHalfDistance</i> parameter, ultimately reaching 100% when all recorded histogram
 values are exhausted.
 
 <a name="linearValues"></a>
 - `histogram.linearValues(valueUnitsPerBucket)`: <br/> Used for iterating through histogram values in linear steps. The iteration is performed 
 in steps of valueUnitsPerBucket in size, terminating when all recorded histogram values are exhausted. Note that each 
 iteration "bucket" includes values up to and including the next bucket boundary value.
 
 <a name="logarithmicValues"></a>
 - `histogram.logarithmicValues(valueUnitsInFirstBucket, logBase)`:  <br/> Iterates through histogram values in logarithmically 
 increasing levels. The iteration is performed in steps that start at `valueUnitsInFirstBucket` and increase exponentially 
 according to `logBase`, terminating when all recorded histogram values are exhausted. Note that each iteration "bucket" 
 includes values up to and including the next bucket boundary value.
 
 <a name="recordedValues"></a>
 - `histogram.recordedValues()`:  <br/> An iterator that enumerate over all non-zero values.
 
  <a name="allValues"></a>
 - `histogram.allValues()`:  <br/>
 Iterates through histogram values using the finest granularity steps supported by the underlying representation.

The iterator methods support the `es6` iterator protocol, so enumeration is typically done with a for-of loop statement. E.g.:

``` javascript
 for (let v of histogram.percentileValues(ticksPerHalfDistance)) {
     ...
 }
```

 or

``` javascript
 for (let v of histogram.linearBucketValues(unitsPerBucket)) {
     ...
 }
```


The loop iteration value is an `object` with the following fields in the general case: 
 * <b><code>value</code></b> : The actual value level that was iterated to by the iterator
 * <b><code>valueIteratedTo</code></b> : The actual value level that was iterated to by the iterator
 * <b><code>valueIteratedFrom</code></b> : The actual value level that was iterated from by the iterator
 * <b><code>count</code></b> : The count of recorded values in the histogram that
 exactly match this [`lowestEquivalentValue(valueIteratedTo)`...`highestEquivalentValue(valueIteratedTo)`] value range.
 * <b><code>countAddedThisIteration</code></b> : The count of recorded values that
    were added as a result on this iteration step. Since multiple iteration
    steps may occur with overlapping equivalent value ranges, the count may be lower than the count found at
    the value (e.g. multiple linear steps or percentile levels can occur within a single equivalent value range)
 * <b><code>lowestEquivalentValue</code></b> : The lowest value that is equivalent to the current iteration 
 value within the histogram's resolution.
 * <b><code>medianEquivalentValue</code></b> : The lowest value that is equivalent to the current iteration 
  value within the histogram's resolution.
 * <b><code>highestEquivalentValue</code> : The highest value that is equivalent to the current iteration 
  value within the histogram's resolution.
 * <b><code>medianEquivalentValue</code></b> : a value that lies in the middle (rounded up) of the range of values 
 (`lowestEquivalentValue` ... `highestEquivalentValue`). "Equivalent" means that value samples recorded for any two equivalent values are counted in a common total count.
 
 In addition to the above, the `percentileValues` iterator returns the following fields:
 * <b><code>cumulativeCount</code></b> : The total count of all recorded values in the histogram at values 
    equal or smaller than `valueIteratedTo`.
 * <b><code>percentile</code></b> : The percentile of recorded values in the histogram at values equal or smaller 
 than `valueIteratedTo`.

 
 
## Acknowledgements

This project was kindly sponsored by [nearForm](http://nearform.com).

The pre-compilation work of this project is only possible because of [mapbox's][mapbox]
amazing work on [node-pre-gyp][node-pre-gyp]. A lot of the functionality enabled
is following the example set by their [node-sqlite3 library][sqlite3].

## License

This library is licensed as MIT

HdrHistogram_c is licensed as [BSD license][HdrHistogram_c-license]

zlib is licensed as [zlib License][zlib-license]

The scripts used in the scripts folder are modified [BSD licensed][sqlite3-scripts-license] scripts from the [node-sqlite3][sqlite3] libary.

[hdr]: http://hdrhistogram.org/
[cimpl]: https://github.com/HdrHistogram/HdrHistogram_c
[node-gyp]: https://github.com/nodejs/node-gyp#installation
[mapbox]: http://mapbox.com
[node-pre-gyp]: https://github.com/mapbox/node-pre-gyp
[sqlite3]: https://github.com/mapbox/node-sqlite3
[HdrHistogram_c-license]: https://github.com/HdrHistogram/HdrHistogram_c/blob/master/LICENSE.txt
[sqlite3-scripts-license]: https://github.com/mapbox/node-sqlite3/blob/master/LICENSE
[zlib-license]: http://www.zlib.net/zlib_license.html
