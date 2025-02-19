/* **********************************************************
 * Copyright (c) 2015-2020 Google, Inc.  All rights reserved.
 * **********************************************************/

/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Google, Inc. nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL VMWARE, INC. OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/* caching_device_stats: represents a hardware caching device.
 */

#ifndef _CACHING_DEVICE_STATS_H_
#define _CACHING_DEVICE_STATS_H_ 1

#include "caching_device_block.h"
#include <string>
#include <map>
#include <stdint.h>
#ifdef HAS_ZLIB
#    include <zlib.h>
#endif
#include "memref.h"

enum invalidation_type_t {
    INVALIDATION_INCLUSIVE,
    INVALIDATION_COHERENCE,
};

enum class metric_name_t {
    HITS,
    MISSES,
    HITS_AT_RESET,
    MISSES_AT_RESET,
    CHILD_HITS,
    CHILD_HITS_AT_RESET,
    INCLUSIVE_INVALIDATES,
    COHERENCE_INVALIDATES,
    PREFETCH_HITS,
    PREFETCH_MISSES,
    FLUSHES
};

class caching_device_stats_t {
public:
    explicit caching_device_stats_t(const std::string &miss_file,
                                    bool warmup_enabled = false,
                                    bool is_coherent = false);
    virtual ~caching_device_stats_t();

    // Called on each access.
    // A multi-block memory reference invokes this routine
    // separately for each block touched.
    virtual void
    access(const memref_t &memref, bool hit, caching_device_block_t *cache_block);

    // Called on each access by a child caching device.
    virtual void
    child_access(const memref_t &memref, bool hit, caching_device_block_t *cache_block);

    virtual void
    print_stats(std::string prefix);

    virtual void
    reset();

    virtual bool operator!()
    {
        return !success_;
    }

    // Process invalidations due to cache inclusions or external writes.
    virtual void
    invalidate(invalidation_type_t invalidation_type);

    int_least64_t
    get_metric(metric_name_t metric) const
    {
        if (stats_map_.find(metric) != stats_map_.end()) {
            return stats_map_.at(metric);
        } else {
            ERRMSG("Wrong metric name.\n");
            return 0;
        }
    }

protected:
    bool success_;

    // print different groups of information, beneficial for code reuse
    virtual void
    print_warmup(std::string prefix);
    virtual void
    print_counts(std::string prefix); // hit/miss numbers
    virtual void
    print_rates(std::string prefix); // hit/miss rates
    virtual void
    print_child_stats(std::string prefix); // child/total info

    virtual void
    dump_miss(const memref_t &memref);

    int_least64_t num_hits_;
    int_least64_t num_misses_;
    int_least64_t num_child_hits_;

    int_least64_t num_inclusive_invalidates_;
    int_least64_t num_coherence_invalidates_;

    // Stats saved when the last reset was called. This helps us get insight
    // into what the stats were when the cache was warmed up.
    int_least64_t num_hits_at_reset_;
    int_least64_t num_misses_at_reset_;
    int_least64_t num_child_hits_at_reset_;
    // Enabled if options warmup_refs > 0 || warmup_fraction > 0
    bool warmup_enabled_;

    // Print out write invalidations if cache is coherent.
    bool is_coherent_;

    // References to the properties with statistics are held in the map with the
    // statistic name as the key. Sample map element: {HITS, num_hits_}
    std::map<metric_name_t, int_least64_t &> stats_map_;

    // We provide a feature of dumping misses to a file.
    bool dump_misses_;
#ifdef HAS_ZLIB
    gzFile file_;
#else
    FILE *file_;
#endif
};

#endif /* _CACHING_DEVICE_STATS_H_ */
