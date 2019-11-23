#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "hdr_histogram.h"
#include "hdr_histogram_ex.h"

bool hdr_equals(struct hdr_histogram* a, struct hdr_histogram* b)
{
    int64_t a_max, b_max, a_min, b_min;
    size_t a_size, b_size, counts_size;

    if (a == b) 
    {
      return true;
    }

    if (a->highest_trackable_value != b->highest_trackable_value ||
        a->lowest_trackable_value != b->lowest_trackable_value ||
        a->significant_figures != b->significant_figures)
    {
      return false;
    }

    if (a->total_count != b->total_count) {
      return false;
    }

    a_max = hdr_max(a);
    b_max = hdr_max(b);

    if (a_max != b_max)
    {
      return false;
    }

    a_min = hdr_min(a);
    b_min = hdr_min(b);

    if (a_min != b_min)
    {
      return false;
    }

    a_size = hdr_get_memory_size(a);
    b_size = hdr_get_memory_size(b);

    if (a_size != b_size)
    {
      return false;
    }

    counts_size = a->counts_len * sizeof(int64_t);

    if (memcmp(a->counts, b->counts, counts_size) == 0)
    {
      return true;
    }

    return false;
}
