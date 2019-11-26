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

int64_t hdr_highest_equivalent_value(const struct hdr_histogram *h, int64_t value) 
{
    return hdr_next_non_equivalent_value(h, value) - 1;
}

int64_t hdr_count_between_values(const struct hdr_histogram *h, int64_t low_value, int64_t high_value) 
{
    int64_t lowest = hdr_lowest_equivalent_value(h, low_value);
    int64_t highest = hdr_highest_equivalent_value(h, high_value);
    int64_t count = 0;
    for (int64_t value = lowest; value <= highest; value = hdr_next_non_equivalent_value(h, value)) {
        count += hdr_count_at_value(h, value);
    }
    return count;
}
