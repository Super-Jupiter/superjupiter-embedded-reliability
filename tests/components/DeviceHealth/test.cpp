#include <SuperJupiter/EmbeddedReliability/DeviceHealth.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <new>
static unsigned long checks = 0;
#define CHECK(x)                                                                                   \
  do {                                                                                             \
    ++checks;                                                                                      \
    if (!(x)) {                                                                                    \
      std::fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x);                                    \
      std::abort();                                                                                \
    }                                                                                              \
  } while (false)
static bool near(double a, double b) { return std::fabs(a - b) <= 1e-9 * (1 + std::fabs(b)); }
using namespace superjupiter::device_health;
void *operator new(std::size_t) { std::abort(); }
void *operator new[](std::size_t) { std::abort(); }
void operator delete(void *p) noexcept { std::free(p); }
void operator delete[](void *p) noexcept { std::free(p); }
int main() {
  (void)near;
  DeviceHealth d(100, 10, 1000);
  CHECK(d.snapshot(0).status == Status::NoSample);
  d.recordSample(0);
  d.recordSample(100);
  d.recordSample(250);
  d.record(Counter::StorageFailures);
  d.observeQueue(4);
  d.observeQueue(2);
  Snapshot s = d.snapshot(300);
  CHECK(s.samples == 3 && s.gaps == 1 && s.largestIntervalMs == 150 && s.sampleAgeMs == 50);
  CHECK(s.storageFailures == 1 && s.queueHighWater == 4);
  CHECK(d.snapshot(1251).status == Status::Stale);
  CHECK(d.snapshot(200).status == Status::InvalidTime);
  CHECK(d.recordSample(250) == Status::InvalidTime);
  CHECK(DeviceHealth(0, 0, 1).recordSample(0) == Status::InvalidConfig);

  DeviceHealth stream(100, 0, 1000);
  for (uint32_t i = 0; i < 1000; ++i)
    CHECK(stream.recordSample(i * 100) == Status::Ok);
  Snapshot final = stream.snapshot(99900);
  CHECK(final.samples == 1000 && final.gaps == 0 && final.largestIntervalMs == 100);
  CHECK(DeviceHealth(0x7fffffffUL, 1, 100).snapshot(0).status == Status::InvalidConfig);

  std::printf("PASS SuperJupiterDeviceHealth: %lu checks\n", checks);
}
