// Consolidated public release baseline: SuperJupiterConfigStore 0.1.1.
#if defined(SUPERJUPITER_CONFIG_STORE_H)
#error "Duplicate SuperJupiter module: use legacy OR consolidated headers for this component in one application"
#endif
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SuperJupiter
#ifndef SUPERJUPITER_EMBEDDEDRELIABILITY_CONFIGSTORE_H
#define SUPERJUPITER_EMBEDDEDRELIABILITY_CONFIGSTORE_H
#if defined(__FAST_MATH__) ||                                                  \
    (defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__ > 0)
#error "SuperJupiter requires IEEE non-finite checks; disable fast-math"
#endif
#include <stddef.h>
#include <stdint.h>
#define SUPERJUPITER_CONFIG_STORE_VERSION "0.1.1"
namespace superjupiter {
namespace config_store {

// Storage adapters must bounds-check, report failures and implement real
// durability in sync().
class Storage {
public:
  virtual bool read(size_t offset, uint8_t *destination, size_t length) = 0;
  virtual bool write(size_t offset, const uint8_t *source, size_t length) = 0;
  virtual bool sync() = 0;
  virtual ~Storage() {}
};
inline uint32_t checksum(const uint8_t *bytes, size_t length) {
  uint32_t crc = 0xffffffffUL;
  for (size_t i = 0; i < length; ++i) {
    crc ^= bytes[i];
    for (unsigned j = 0; j < 8; ++j)
      crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320UL : 0);
  }
  return ~crc;
}
inline void put32(uint8_t *p, uint32_t value) {
  for (unsigned i = 0; i < 4; ++i)
    p[i] = static_cast<uint8_t>(value >> (8 * i));
}
inline uint32_t get32(const uint8_t *p) {
  uint32_t value = 0;
  for (unsigned i = 0; i < 4; ++i)
    value |= static_cast<uint32_t>(p[i]) << (8 * i);
  return value;
}

enum class Status {
  Ok,
  Empty,
  Recovered,
  SchemaMismatch,
  Corrupt,
  IoError,
  NotLoaded,
  Overflow,
  StaleState
};
template <size_t Size> class ConfigStore {
  static_assert(Size > 0 && Size <= 1024,
                "ConfigStore requires 1..1024 payload bytes");
  static const size_t Bytes = Size + 25;
  Storage &storage_;
  size_t base_;
  uint32_t schema_;
  uint64_t generation_;
  int active_;
  bool ready_;
  static uint64_t generation(const uint8_t *b) {
    return get32(b + 4) | (static_cast<uint64_t>(get32(b + 8)) << 32);
  }
  bool valid(const uint8_t *b) const {
    return b[Bytes - 1] == 0xa5 && b[0] == 0x53 && b[1] == 0x43 && b[2] == 1 &&
           b[3] == 0 && get32(b + 16) == Size && generation(b) > 0 &&
           get32(b + 20 + Size) == checksum(b, 20 + Size);
  }

public:
  ConfigStore(Storage &storage, uint32_t schema, size_t base = 0)
      : storage_(storage), base_(base), schema_(schema), generation_(0),
        active_(-1), ready_(false) {}
  static size_t requiredBytes() { return 2 * Bytes; }
  uint64_t revision() const { return generation_; }
  Status load(uint8_t *output) {
    ready_ = false;
    if (!output || base_ > SIZE_MAX - requiredBytes())
      return Status::IoError;
    uint8_t a[Bytes], b[Bytes];
    if (!storage_.read(base_, a, Bytes) ||
        !storage_.read(base_ + Bytes, b, Bytes))
      return Status::IoError;
    const bool va = valid(a), vb = valid(b);
    const bool badA = a[Bytes - 1] != 0 && a[Bytes - 1] != 255 && !va,
               badB = b[Bytes - 1] != 0 && b[Bytes - 1] != 255 && !vb;
    if (!va && !vb) {
      generation_ = 0;
      active_ = -1;
      if (badA || badB)
        return Status::Corrupt;
      ready_ = true;
      return Status::Empty;
    }
    if (va && vb && generation(a) == generation(b))
      return Status::Corrupt;
    active_ = vb && (!va || generation(b) > generation(a)) ? 1 : 0;
    const uint8_t *chosen = active_ ? b : a;
    generation_ = generation(chosen);
    if (get32(chosen + 12) != schema_)
      return Status::SchemaMismatch;
    for (size_t i = 0; i < Size; ++i)
      output[i] = chosen[20 + i];
    ready_ = true;
    return badA || badB ? Status::Recovered : Status::Ok;
  }
  Status save(const uint8_t *input) {
    if (!ready_)
      return Status::NotLoaded;
    if (!input)
      return Status::IoError;
    if (generation_ == UINT64_MAX)
      return Status::Overflow;
    uint8_t a[Bytes], existingB[Bytes];
    if (!storage_.read(base_, a, Bytes) ||
        !storage_.read(base_ + Bytes, existingB, Bytes)) {
      ready_ = false;
      return Status::IoError;
    }
    const bool va = valid(a), vb = valid(existingB);
    const bool badA = a[Bytes - 1] != 0 && a[Bytes - 1] != 255 && !va;
    const bool badB =
        existingB[Bytes - 1] != 0 && existingB[Bytes - 1] != 255 && !vb;
    if (!va && !vb && (badA || badB)) {
      ready_ = false;
      return Status::Corrupt;
    }
    if (va && vb && generation(a) == generation(existingB)) {
      ready_ = false;
      return Status::Corrupt;
    }
    const int newest = vb && (!va || generation(existingB) > generation(a))
                           ? 1
                           : (va ? 0 : -1);
    const uint64_t latest =
        newest == 0 ? generation(a) : (newest == 1 ? generation(existingB) : 0);
    if (newest != active_ || latest != generation_) {
      ready_ = false;
      return Status::StaleState;
    }
    const int slot = active_ == 0 ? 1 : 0;
    const size_t offset = base_ + static_cast<size_t>(slot) * Bytes;
    const uint64_t next = generation_ + 1;
    uint8_t b[Bytes] = {};
    b[0] = 0x53;
    b[1] = 0x43;
    b[2] = 1;
    put32(b + 4, static_cast<uint32_t>(next));
    put32(b + 8, static_cast<uint32_t>(next >> 32));
    put32(b + 12, schema_);
    put32(b + 16, Size);
    for (size_t i = 0; i < Size; ++i)
      b[20 + i] = input[i];
    put32(b + 20 + Size, checksum(b, 20 + Size));
    const uint8_t invalid = 0, commit = 0xa5;
    ready_ = false;
    if (!storage_.write(offset + Bytes - 1, &invalid, 1) || !storage_.sync() ||
        !storage_.write(offset, b, Bytes - 1) || !storage_.sync() ||
        !storage_.write(offset + Bytes - 1, &commit, 1) || !storage_.sync())
      return Status::IoError;
    active_ = slot;
    generation_ = next;
    ready_ = true;
    return Status::Ok;
  }
};
} // namespace config_store
} // namespace superjupiter
#endif
