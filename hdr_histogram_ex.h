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

#endif