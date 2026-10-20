// Consolidated public release baseline: SuperJupiterDeviceHealth 0.1.1.
#if defined(SUPERJUPITER_DEVICE_HEALTH_H)
#error "Duplicate SuperJupiter module: use legacy OR consolidated headers for this component in one application"
#endif
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SuperJupiter
#ifndef SUPERJUPITER_EMBEDDEDRELIABILITY_DEVICEHEALTH_H
#define SUPERJUPITER_EMBEDDEDRELIABILITY_DEVICEHEALTH_H
#if defined(__FAST_MATH__) ||                                                  \
    (defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__ > 0)
#error "SuperJupiter requires IEEE non-finite checks; disable fast-math"
#endif
#include <stddef.h>
#include <stdint.h>
#define SUPERJUPITER_DEVICE_HEALTH_VERSION "0.1.1"
namespace superjupiter {
namespace device_health {

enum class Status { Ok, NoSample, Stale, InvalidTime, InvalidConfig };
enum class Counter {
  DroppedRecords,
  StorageFailures,
  UploadFailures,
  Restarts
};
struct Snapshot {
  Status status;
  uint32_t sampleAgeMs, largestIntervalMs;
  uint64_t samples, gaps, droppedRecords, storageFailures, uploadFailures,
      restarts;
  size_t queueHighWater;
  bool saturated;
};
class DeviceHealth {
  uint32_t expected_, tolerance_, stale_, last_, largest_;
  bool seen_, saturated_;
  uint64_t samples_, gaps_, counters_[4];
  size_t high_;
  void increment(uint64_t &value) {
    if (value == UINT64_MAX)
      saturated_ = true;
    else
      ++value;
  }

public:
  DeviceHealth(uint32_t expectedIntervalMs, uint32_t toleranceMs,
               uint32_t staleAfterMs)
      : expected_(expectedIntervalMs), tolerance_(toleranceMs),
        stale_(staleAfterMs), last_(0), largest_(0), seen_(false),
        saturated_(false), samples_(0), gaps_(0), counters_{}, high_(0) {}
  bool valid() const {
    return expected_ > 0 &&
           static_cast<uint64_t>(expected_) + tolerance_ < 0x80000000ULL &&
           stale_ > 0 && stale_ < 0x80000000UL;
  }
  Status recordSample(uint32_t atMs) {
    if (!valid())
      return Status::InvalidConfig;
    if (seen_) {
      const uint32_t dt = atMs - last_;
      if (!dt || dt >= 0x80000000UL)
        return Status::InvalidTime;
      if (dt > largest_)
        largest_ = dt;
      if (dt > static_cast<uint64_t>(expected_) + tolerance_)
        increment(gaps_);
    }
    seen_ = true;
    last_ = atMs;
    increment(samples_);
    return Status::Ok;
  }
  void record(Counter counter) {
    const unsigned i = static_cast<unsigned>(counter);
    if (i < 4)
      increment(counters_[i]);
  }
  void observeQueue(size_t depth) {
    if (depth > high_)
      high_ = depth;
  }
  Snapshot snapshot(uint32_t nowMs) const {
    Snapshot r = {Status::NoSample,
                  0,
                  largest_,
                  samples_,
                  gaps_,
                  counters_[0],
                  counters_[1],
                  counters_[2],
                  counters_[3],
                  high_,
                  saturated_};
    if (!valid())
      r.status = Status::InvalidConfig;
    else if (seen_) {
      r.sampleAgeMs = nowMs - last_;
      r.status = r.sampleAgeMs >= 0x80000000UL
                     ? Status::InvalidTime
                     : (r.sampleAgeMs > stale_ ? Status::Stale : Status::Ok);
    }
    return r;
  }
};
} // namespace device_health
} // namespace superjupiter
#endif
