// SPDX-License-Identifier: MIT
#include <SuperJupiter/EmbeddedReliability/ConfigStore.h>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
static unsigned checks = 0;
#define CHECK(x)                                                               \
  do {                                                                         \
    ++checks;                                                                  \
    if (!(x)) {                                                                \
      std::fprintf(stderr, "FAIL %s:%d %s\n", __FILE__, __LINE__, #x);         \
      std::abort();                                                            \
    }                                                                          \
  } while (false)
static bool close(double a, double b) {
  return std::fabs(a - b) <= 1e-12 * (1 + std::fabs(b));
}
using namespace superjupiter::config_store;

class Memory : public Storage {
public:
  uint8_t bytes[256];
  Memory() {
    for (size_t i = 0; i < sizeof(bytes); ++i)
      bytes[i] = 255;
  }
  bool read(size_t o, uint8_t *p, size_t n) {
    if (o > sizeof(bytes) || n > sizeof(bytes) - o)
      return false;
    for (size_t i = 0; i < n; ++i)
      p[i] = bytes[o + i];
    return true;
  }
  bool write(size_t o, const uint8_t *p, size_t n) {
    if (o > sizeof(bytes) || n > sizeof(bytes) - o)
      return false;
    for (size_t i = 0; i < n; ++i)
      bytes[o + i] = p[i];
    return true;
  }
  bool sync() { return true; }
};

int main() {
  (void)close;

  Memory storage;
  uint8_t first[4] = {1, 2, 3, 4}, second[4] = {2, 3, 4, 5},
          third[4] = {3, 4, 5, 6}, out[4] = {};
  ConfigStore<4> a(storage, 1), b(storage, 1);
  CHECK(a.load(out) == Status::Empty);
  CHECK(a.save(first) == Status::Ok);
  CHECK(b.load(out) == Status::Ok);
  CHECK(a.save(second) == Status::Ok);
  CHECK(b.save(third) != Status::Ok);
  ConfigStore<4> restored(storage, 1);
  CHECK(restored.load(out) == Status::Ok && out[0] == 2 &&
        restored.revision() == 2);
  Memory damaged;
  ConfigStore<4> empty(damaged, 1);
  CHECK(empty.load(out) == Status::Empty);
  damaged.bytes[28] = 0xa5;
  CHECK(empty.save(first) == Status::Corrupt);
  CHECK(empty.save(first) == Status::NotLoaded);
  CHECK(damaged.bytes[0] == 255);

  std::printf("PASS SuperJupiterConfigStore review: %u checks\n", checks);
}
