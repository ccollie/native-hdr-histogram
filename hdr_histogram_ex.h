#ifndef HDR_HISTOGRAM_EX_H
#define HDR_HISTOGRAM_EX_H

#include "hdr_histogram.h"

/**
 * Determine if 2 histograms are equivalent (having similar parameters and values).
 *
 * @param h "This" pointer
 * @param other histogram to compare to
 * @return true if the histograms are equivalent, false if not
 */
bool hdr_equals(struct hdr_histogram* h, struct hdr_histogram* other);

/**
 * Get the highest value that is equivalent to the given value within the histogram's resolution.
 * Where "equivalent" means that value samples recorded for any two
 * equivalent values are counted in a common total count.
 *
 * @param h "This" pointer
 * @param value The given value
 * @return The highest value that is equivalent to the given value within the histogram's resolution.
 */
int64_t hdr_highest_equivalent_value(const struct hdr_histogram *h, int64_t value);

 /**
 * Get the count of recorded values within a range of value levels (inclusive to within the histogram's resolution).
 *
 * @param low_value  The lower value bound on the range for which to provide the recorded count. Will be rounded down with
 *                  {@link hdr_lowest_equivalent_value lowestEquivalentValue}.
 * @param high_value  The higher value bound on the range for which to provide the recorded count.
 *                   Will be rounded up with {@link hdr_highest_equivalent_value highestEquivalentValue}.
 * @return the total count of values recorded in the histogram within the value range that is
 * {@literal >=} lowestEquivalentValue(<i>lowValue</i>) and {@literal <=} highestEquivalentValue(<i>highValue</i>)
 */
int64_t hdr_count_between_values(const struct hdr_histogram* h, int64_t low_value, int64_t high_value);

#endif