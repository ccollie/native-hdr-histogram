'use strict'
const test = require('tap').test
const Histogram = require('./')

test('create an histogram', (t) => {
  t.doesNotThrow(() => new Histogram(1, 100))
  t.end()
})

test('create an histogram with a constructor', (t) => {
  t.doesNotThrow(() => new Histogram(1, 100))
  t.end()
})

test('create an histogram arguments checks', (t) => {
  t.throws(() => new Histogram(-1, 100))
  t.throws(() => new Histogram(0, 100))
  t.throws(() => new Histogram(1, 100, 20))
  t.throws(() => new Histogram(1, 100, 0))
  t.throws(() => new Histogram(1, 100, 6))
  for (let i = 1; i < 5; i++) {
    t.doesNotThrow(() => new Histogram(1, 100, i))
  }
  t.end()
})

test('properties', (t) => {
  const instance = new Histogram(5, 150, 4)

  t.equal(instance.lowestTrackableValue, 5, 'properly returns lowestTrackableValue')
  t.equal(instance.highestTrackableValue, 150, 'properly returns highestTrackableValue')
  t.equal(instance.significantFigures, 4, 'properly returns significantFigures')
  t.equal(instance.totalCount, 0, 'properly returns totalCount')
  t.equal(instance.memorySize > 0, true, 'memory allocated for instance')
  t.end()
})

test('totalCount', (t) => {
  const instance = new Histogram(1, 100)
  const count = Math.floor(Math.random() * 20) + 10
  for (let i = 0; i < count; i++) {
    instance.record(i + 1)
  }
  t.equal(instance.totalCount, count, 'totalCount is correct after add')
  t.ok(instance.reset())
  t.equal(instance.totalCount, 0, 'totalCount is correct after reset')
  t.end()
})

test('record values in an histogram', (t) => {
  const instance = new Histogram(1, 100)
  t.ok(instance.record(42))
  t.ok(instance.record(45))
  t.equal(instance.totalCount, 2, 'recordValue increments totalCount')
  t.end()
})

test('recording a non-value returns false', (t) => {
  const instance = new Histogram(1, 100)
  t.notOk(instance.record())
  t.notOk(instance.record(-42))
  t.end()
})

test('record values with counts', (t) => {
  const testValueLevel = 4

  const multiplier = 10
  const instance = new Histogram(1, 360000, 3)
  for (let i = 1; i < 5; i++) {
    instance.record(i, multiplier)
    t.equal(i * multiplier, instance.totalCount, 'record(value, count) increments totalCount')
  }
  t.ok(instance.reset())

  for (let j = 1; j < 5; j++) {
    instance.record(testValueLevel, multiplier)
    t.equal(j * multiplier, instance.countAtValue(testValueLevel), 'record(value, count) increments countAtValue')
  }

  t.end()
})

test('recordCorrectedValue', (t) => {
  const INTERVAL = 10000
  const HIGHEST = 3600 * 1000 * 1000
  const SIGNIFICANT = 3
  const instance = new Histogram(1, HIGHEST, SIGNIFICANT)

  // record this value with a count of 10,000
  instance.recordCorrectedValue(1000, INTERVAL, 10000)
  instance.recordCorrectedValue(100000000, INTERVAL)

  function checkPercentile (percentile, value, tolerance) {
    const valueAt = instance.percentile(percentile)
    const diff = Math.abs(valueAt - value)
    const val = value * tolerance
    t.ok(diff < val)
  }

  const percentiles = [
    [30.0, 1000.0],
    [50.0, 1000.0],
    [75.0, 50000000.0],
    [90.0, 80000000.0],
    [99.0, 98000000.0],
    [99.999, 100000000.0],
    [100.0, 100000000.0]
  ]

  percentiles.forEach(pair => {
    checkPercentile(pair[0], pair[1], 0.001)
  })
  t.equal(instance.totalCount, 20000, 'counts match')
  t.ok(instance.valuesAreEquivalent(instance.min(), 1000.0))
  t.ok(instance.valuesAreEquivalent(instance.max(), 100000000.0))
  t.end()
})

test('add histogram', (t) => {
  const highestTrackableValue = 3600 * 1000 * 1000 // 1 hour in usec units
  const numberOfSignificantValueDigits = 3
  const testValueLevel = 4

  const histogram = new Histogram(1, highestTrackableValue, numberOfSignificantValueDigits)
  const other = new Histogram(1, highestTrackableValue, numberOfSignificantValueDigits)

  histogram.record(testValueLevel)
  histogram.record(testValueLevel * 1000)
  other.record(testValueLevel)
  other.record(testValueLevel * 1000)

  histogram.add(other)
  t.equal(histogram.totalCount, 4, 'add should increase the totalCount of the destination by the count added')

  const biggerOther = new Histogram(1, highestTrackableValue * 2, numberOfSignificantValueDigits)
  biggerOther.record(testValueLevel)
  biggerOther.record(testValueLevel * 1000)
  biggerOther.record(highestTrackableValue * 2)

  // Adding the smaller histogram to the bigger one should work:
  const dropped = biggerOther.add(histogram)
  t.equal(dropped, 0, 'no values should be dropped if a histogram with a smaller range is added')
  t.equal(biggerOther.totalCount, 7)

  // But trying to add a larger histogram into a smaller one should cause values to be dropped
  t.equal(histogram.add(biggerOther), 1, 'add a larger histogram to a smaller one causes out of range values to be dropped')

  t.end()
})

test('fail add', (t) => {
  const instance = new Histogram(1, 100)
  t.throws(() => instance.add())
  t.throws(() => instance.add('hello'))
  t.throws(() => instance.add({}))
  t.end()
})

test('add with subclasses', (t) => {
  class Subclass extends Histogram {
    constructor () {
      super(1, 100)
    }
  }

  const instance = new Histogram(1, 100)
  const sub = new Subclass()
  for (let i = 0; i < 5; i++) {
    sub.record((i + 1) * 10)
  }
  t.equal(instance.add(sub), 0, 'can add from subclass')
  t.equal(instance.totalCount, sub.totalCount, 'totalCount is correct after add')
  t.end()
})

test('equals', (t) => {
  let first = new Histogram(1, 100, 2)
  let second = new Histogram(1, 100, 2)

  t.ok(first.equals(first), 'A histogram should be equal to itself')

  t.ok(first.equals(second), 'Empty histograms with similar parameters should be equal')
  first.record(20)
  first.record(42)

  t.notOk(first.equals(second), 'Histograms with different counts are different')
  second.record(42)
  t.notOk(first.equals(second), 'Histograms with different counts are different')
  second.record(20)
  t.ok(first.equals(second), 'Histograms with equal counts are equal')

  second = new Histogram(5, 100, 3)
  t.notOk(first.equals(second), 'Histograms with different parameters are not equal')
  second.record(20)
  second.record(42)

  t.notOk(first.equals(second), 'Histograms with different parameters are not equal even if counts are equal')

  t.end()
})

test('copy', (t) => {
  let first = new Histogram(1, 100, 2)
  let second = first.copy()
  t.ok(first.equals(second), 'Can create an empty copy')
  first.record(5)
  first.record(20)
  first.record(43)

  t.notOk(first.equals(second), 'Copies are separate instances and do not share references')
  second = first.copy()
  t.ok(second.equals(first), 'Copies data')

  t.end()
})

test('equals with non-histograms', (t) => {
  let first = new Histogram(1, 100, 2)
  t.throws(() => first.equals(1))
  t.throws(() => first.equals([]))
  t.throws(() => first.equals({}))
  t.throws(() => first.equals(null))
  t.end()
})

test('stdev, mean, min, max', (t) => {
  const instance = new Histogram(1, 100)
  t.ok(instance.record(42))
  t.ok(instance.record(45))
  t.equal(instance.min(), 42, 'min is available')
  t.equal(instance.max(), 45, 'max is available')
  t.equal(instance.mean(), 43.5, 'mean is available')
  t.equal(instance.stddev(), 1.5, 'stdev is available')
  t.end()
})

test('percentile', (t) => {
  const instance = new Histogram(1, 100)
  t.ok(instance.record(42))
  t.ok(instance.record(42))
  t.ok(instance.record(45))
  t.equal(instance.percentile(10), 42, 'percentile match')
  t.equal(instance.percentile(99), 45, 'percentile match')
  t.equal(instance.percentile(100), 45, 'percentile match')
  t.end()
})

test('wrong percentile', (t) => {
  const instance = new Histogram(1, 100)
  t.ok(instance.record(42))
  t.ok(instance.record(42))
  t.ok(instance.record(45))
  t.throws(() => instance.percentile(), 'no percentile throws')
  t.throws(() => instance.percentile(101), 'percentile > 100 throws')
  t.throws(() => instance.percentile(0), 'percentile == 0 throws')
  t.throws(() => instance.percentile(-1), 'percentile < 0 throws')
  t.end()
})

test('encode/decode', (t) => {
  const instance = new Histogram(1, 100)
  t.ok(instance.record(42))
  t.ok(instance.record(42))
  t.ok(instance.record(45))
  const instance2 = Histogram.decode(instance.encode())
  t.equal(instance2.percentile(10), 42, 'percentile match')
  t.equal(instance2.percentile(99), 45, 'percentile match')
  t.end()
})

test('fail decode', (t) => {
  t.throws(() => Histogram.decode())
  t.throws(() => Histogram.decode('hello'))
  t.throws(() => Histogram.decode({}))
  t.throws(() => Histogram.decode(42))
  t.end()
})

test('percentiles', (t) => {
  const instance = new Histogram(1, 100)
  t.deepEqual(instance.percentiles(), [{
    percentile: 100,
    value: 0
  }], 'empty percentiles has 0 till 100%')
  t.ok(instance.record(42))
  t.ok(instance.record(42))
  t.ok(instance.record(45))
  t.deepEqual(instance.percentiles(), [{
    percentile: 0,
    value: 42
  }, {
    percentile: 50,
    value: 42
  }, {
    percentile: 75,
    value: 45
  }, {
    percentile: 100,
    value: 45
  }], 'percentiles matches')
  t.end()
})

test('support >2e9', (t) => {
  const recordValue = 4 * 1e9
  const instance = new Histogram(1, recordValue)
  var compare = (a, b) => {
    var diff = Math.abs(a - b)
    // hdr_min and hdr_max do not return precise data, even before
    // conversion to double.
    if (diff < 1e-3 * Math.min(Math.abs(a), Math.abs(b))) {
      return true
    } else {
      console.error(`Mismatch! Got ${a}, expected ${b}!`)
      return false
    }
  }
  t.ok(instance.record(recordValue))
  t.ok(compare(instance.min(), recordValue), 'min match')
  t.ok(compare(instance.max(), recordValue), 'max match')
  t.end()
})

test('countAtValue', (t) => {
  const instance = new Histogram(1, 100)
  const testValueLevel = 4
  const count = Math.floor(Math.random() * 20) + 10
  for (let i = 0; i < count; i++) {
    instance.record(testValueLevel)
  }
  t.equal(instance.countAtValue(testValueLevel), count, 'record increments countAtValue')
  t.ok(instance.reset())
  t.equal(instance.countAtValue(testValueLevel), 0, 'reset() sets count to zero')
  t.end()
})

test('countBetweenValues', (t) => {
  const highestTrackableValue = 3600 * 1000 * 1000 // 1 hour in usec units
  const numberOfSignificantValueDigits = 3 // Maintain at least 3 decimal points of accuracy

  const histogram = new Histogram(1, highestTrackableValue, numberOfSignificantValueDigits)
  const rawHistogram = new Histogram(1, highestTrackableValue, numberOfSignificantValueDigits)

  // Log hypothetical scenario: 100 seconds of "perfect" 1msec results, sampled
  // 100 times per second (10,000 results), followed by a 100 second pause with
  // a single (100 second) recorded result. Recording is done indicating an expected
  // interval between samples of 10 msec:
  for (let i = 0; i < 10000; i++) {
    histogram.recordCorrectedValue(1000 /* 1 msec */, 10000 /* 10 msec expected interval */)
    rawHistogram.record(1000 /* 1 msec */)
  }
  histogram.recordCorrectedValue(100000000 /* 100 sec */, 10000 /* 10 msec expected interval */)
  rawHistogram.record(100000000 /* 100 sec */)

  t.equals(rawHistogram.countBetweenValues(1000, 1000), 10000, 'Count of raw values between 1 msec and 1 msec is 1')
  t.equals(rawHistogram.countBetweenValues(5000, 150000000), 1, 'Count of raw values between 5 msec and 150 sec is 1')
  t.equals(histogram.countBetweenValues(5000, 150000000), 10000, 'Count of values between 5 msec and 150 sec is 10,000')
  t.end()
})

test('valuesAreEquivalent', (t) => {
  const instance = new Histogram(20000000, 100000000, 5)
  instance.record(100000000)
  instance.record(20000000)
  instance.record(30000000)
  t.equal(true, instance.valuesAreEquivalent(20000000, instance.percentile(50.0)))
  t.equal(true, instance.valuesAreEquivalent(30000000, instance.percentile(50.0)))
  t.equal(true, instance.valuesAreEquivalent(100000000, instance.percentile(83.34)))
  t.equal(true, instance.valuesAreEquivalent(100000000, instance.percentile(99.0)))

  t.throws(() => instance.valuesAreEquivalent())
  t.throws(() => instance.valuesAreEquivalent(1000))
  t.end()
})

test('valuesAreEquivalent argument checks', (t) => {
  const instance = new Histogram(1, 1000)
  t.throws(() => instance.valuesAreEquivalent())
  t.throws(() => instance.valuesAreEquivalent(1000))
  t.end()
})

test('lowestEquivalentValue', (t) => {
  const histogram = new Histogram(1, 3600 * 1000 * 1000, 3) // e.g. for 1 hr in usec units;
  t.equal(histogram.lowestEquivalentValue(10007), 10000, 'The lowest equivalent value to 10007 is 10000')
  t.equal(histogram.lowestEquivalentValue(10009), 10008, 'The lowest equivalent value to 10009 is 10008')
  t.end()
})

test('highestEquivalentValue', (t) => {
  const histogram = new Histogram(1024, 3600000000, 3)
  t.equal(histogram.highestEquivalentValue(8180 * 1024), 8183 * 1024 + 1023, 'The highest equivalent value to 8180 * 1024 is 8183 * 1024 + 1023')
  t.equal(histogram.highestEquivalentValue(8191 * 1024), 8191 * 1024 + 1023, 'The highest equivalent value to 8187 * 1024 is 8191 * 1024 + 1023')
  t.equal(histogram.highestEquivalentValue(8193 * 1024), 8199 * 1024 + 1023, 'The highest equivalent value to 8193 * 1024 is 8199 * 1024 + 1023')
  t.equal(histogram.highestEquivalentValue(9995 * 1024), 9999 * 1024 + 1023, 'The highest equivalent value to 9995 * 1024 is 9999 * 1024 + 1023')
  t.equal(histogram.highestEquivalentValue(10007 * 1024), 10007 * 1024 + 1023, 'The highest equivalent value to 10007 * 1024 is 10007 * 1024 + 1023')
  t.equal(histogram.highestEquivalentValue(10008 * 1024), 10015 * 1024 + 1023, 'The highest equivalent value to 10008 * 1024 is 10015 * 1024 + 1023')
  t.end()
})

test('nextNonEquivalentValue', (t) => {
  const histogram = new Histogram(1, 3600 * 1000 * 1000, 3) // e.g. for 1 hr in usec units;
  let prev = histogram.lowestEquivalentValue(10007) // 10000
  for (let i = 0; i < 5; i++) {
    let value = histogram.nextNonEquivalentValue(prev)
    t.notOk(histogram.valuesAreEquivalent(value, prev), 'values should not be equivalent')
    prev = value
  }
  t.end()
})

test('reset histogram', (t) => {
  const instance = new Histogram(1, 100)
  t.equal(instance.min(), 9223372036854776000, 'min is setup')
  t.equal(instance.max(), 0, 'max is setup')
  t.ok(instance.record(42))
  t.ok(instance.record(45))
  t.equal(instance.min(), 42, 'min is correct before reset')
  t.equal(instance.max(), 45, 'max is correct before reset')
  t.equal(instance.mean(), 43.5, 'mean is correct before reset')
  t.equal(instance.stddev(), 1.5, 'stdev is correct before reset')
  t.ok(instance.reset())
  t.equal(instance.min(), 9223372036854776000, 'min is reset')
  t.equal(instance.max(), 0, 'max is reset')
  t.ok(instance.record(52))
  t.ok(instance.record(55))
  t.equal(instance.min(), 52, 'min is correct after reset')
  t.equal(instance.max(), 55, 'max is correct after reset')
  t.equal(instance.mean(), 53.5, 'mean is correct after reset')
  t.equal(instance.stddev(), 1.5, 'stdev is correct after reset')
  // test to ensure maintains ref to self when returning after reset
  var resetInstance = instance.reset()
  t.equal(instance, resetInstance)
  t.end()
})

// Iterators
function loadIteratorHistograms () {
  const highestTrackableValue = 3600 * 1000 * 1000
  const significantFigures = 3
  const interval = 10000

  const rawHistogram = new Histogram(1, highestTrackableValue, significantFigures)
  const corHistogram = new Histogram(1, highestTrackableValue, significantFigures)

  for (let i = 0; i < 10000; i++) {
    rawHistogram.record(1000)
    corHistogram.recordCorrectedValue(1000, interval)
  }

  rawHistogram.record(100000000)
  corHistogram.recordCorrectedValue(100000000, 10000)

  return { rawHistogram, corHistogram }
}

test('allValues() iterator', (t) => {
  const histogram = new Histogram(1, 3600 * 1000 * 1000, 3)
  const rawHistogram = new Histogram(1, 3600 * 1000 * 1000, 3)

  for (let i = 0; i < 10000; i++) {
    histogram.recordCorrectedValue(1000 /* 1 msec */, 10000 /* 10 msec expected interval */)
    rawHistogram.record(1000 /* 1 msec */)
  }
  histogram.recordCorrectedValue(100000000 /* 100 sec */, 10000 /* 10 msec expected interval */)
  rawHistogram.record(100000000 /* 100 sec */)

  let index = 0
  // Iterate raw data by stepping through every value that has a count recorded:
  for (let { value, count } of rawHistogram.allValues()) {
    if (index === 1000) {
      t.equals(count, 10000, 'Raw allValues bucket # 0 added a count of 10000')
    } else if (rawHistogram.valuesAreEquivalent(value, 100000000)) {
      t.equals(count, 1, 'Raw allValues value bucket # ' + index + ' added a count of 1')
    } else {
      t.equals(count, 0, 'Raw allValues value bucket # ' + index + ' added a count of 0')
    }
    index++
  }

  index = 0
  let totalAddedCounts = 0
  let prev = 0
  for (let v of histogram.allValues()) {
    let { countAddedThisIteration, value, count } = v
    if (index === 1000) {
      t.equals(countAddedThisIteration, 10000, 'AllValues bucket # 0 [' + prev + '..' + value + '] added a count of 10000')
    }
    t.equals(count, countAddedThisIteration, 'The count in AllValues bucket #' + index +
            ' is exactly the amount added since the last iteration ')
    totalAddedCounts += countAddedThisIteration
    index++
    prev = value
  }
  t.equals(totalAddedCounts, 20000, 'Total added counts should be 20000')

  t.end()
})

test('recordedValues iterator', (t) => {
  let totalAddedCount = 0

  const { rawHistogram, corHistogram } = loadIteratorHistograms()

  /* Raw Histogram */
  let index = 0
  for (let v of rawHistogram.recordedValues()) {
    let { countAddedThisIteration } = v

    if (index === 0) {
      t.equal(countAddedThisIteration, 10000, 'Value at 0 should be 10000')
    } else {
      t.equal(countAddedThisIteration, 1, 'Value at 1 should be 1')
    }

    index++
  }

  t.equal(index, 2, 'Should have encountered 2 values')

  /* Corrected Histogram */
  index = 0
  for (let v of corHistogram.recordedValues()) {
    let { countAddedThisIteration, count } = v

    if (index === 0) {
      t.equal(countAddedThisIteration, 10000, 'Count at 0 is 10000')
    }
    t.notequal(count, 0, 'Count should not be 0')
    t.equal(count, countAddedThisIteration, 'Count at value iterated to should be count added in this step')
    totalAddedCount += countAddedThisIteration

    index++
  }

  t.equal(totalAddedCount, 20000, 'Total counts should be 20000')
  t.end()
})

test('linearValues iterator', (t) => {
  const { rawHistogram, corHistogram } = loadIteratorHistograms()

  /* Raw Histogram */
  let index = 0
  for (let { countAddedThisIteration } of rawHistogram.linearValues(100000)) {
    if (index === 0) {
      t.equal(countAddedThisIteration, 10000, 'Count at 0 should be 10000')
    } else if (index === 999) {
      t.equal(countAddedThisIteration, 1, 'Count at 999 should be 1')
    } else {
      t.equal(countAddedThisIteration, 0, 'Count should be 0')
    }

    index++
  }
  t.equal(index, 1000, 'Should have met 1000 values')

  /* Corrected Histogram */
  index = 0
  let totalAddedCount = 0
  for (let { countAddedThisIteration } of corHistogram.linearValues(10000)) {
    if (index === 0) {
      t.equal(countAddedThisIteration, 10001, 'Count at 0 is 10001')
    }

    totalAddedCount += countAddedThisIteration
    index++
  }

  t.equal(index, 10000, 'Should have met 10000 values')
  t.equal(totalAddedCount, 20000, 'Should have met 20000 values')

  t.end()
})

test('logarithmicValues iterator', (t) => {
  const { rawHistogram, corHistogram } = loadIteratorHistograms()

  let index = 0

  for (let v of rawHistogram.logarithmicValues(10000, 2.0)) {
    let { countAddedThisIteration } = v

    if (index === 0) {
      t.equal(countAddedThisIteration, 10000, 'Raw Logarithmic 10 msec bucket # 0 added a count of 10000')
    } else if (index === 14) {
      t.equal(countAddedThisIteration, 1, 'Raw Logarithmic 10 msec bucket # 14 added a count of 1')
    } else {
      t.equal(countAddedThisIteration, 0, 'Raw Logarithmic 10 msec bucket added a count of 0')
    }

    index++
  }

  t.equals(index - 1, 14, 'Should have seen 14 values')

  index = 0
  let totalAddedCount = 0
  for (let { countAddedThisIteration } of corHistogram.logarithmicValues(10000, 2.0)) {
    if (index === 0) {
      t.equal(countAddedThisIteration, 10001, 'Corrected Logarithmic 10 msec bucket # 0 added a count of 10001')
    }
    totalAddedCount += countAddedThisIteration
    index++
  }

  t.equal(index - 1, 14, 'Should have seen 14 values')
  t.equal(totalAddedCount, 20000, 'Should have seen a count of 20000')
  t.end()
})

test('percentileValues iterator', (t) => {
  const histogram = new Histogram(1, 3600 * 1000 * 1000, 3)
  // record this value with a count of 10,000
  histogram.record(1000, 10000)
  histogram.record(100000000)

  // test with 5 ticks per half distance
  for (let item of histogram.percentileValues(5)) {
    // fix-me
    if (item.percentile > 0 && item.percentile < 99.99) {
      let valueAtPercentile = histogram.percentile(item.percentile)
      let expected = histogram.highestEquivalentValue(valueAtPercentile)
      if (expected !== item.valueIteratedTo) {
        console.log(item)
      }
      t.equals(expected, item.valueIteratedTo)
    }
  }

  t.end()
})
