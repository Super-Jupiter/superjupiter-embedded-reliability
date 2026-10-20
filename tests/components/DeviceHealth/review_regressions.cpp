// SPDX-License-Identifier: MIT
#include <SuperJupiter/EmbeddedReliability/DeviceHealth.h>
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
using namespace superjupiter::device_health;

int main() {
  (void)close;

  DeviceHealth health(100, 0, 1000);
  health.recordSample(UINT32_MAX - 99);
  CHECK(health.recordSample(0) == Status::Ok);
  health.record(Counter::DroppedRecords);
  health.record(Counter::Restarts);
  health.observeQueue(10);
  Snapshot status = health.snapshot(1001);
  CHECK(status.status == Status::Stale && status.gaps == 0 &&
        status.droppedRecords == 1 && status.restarts == 1 &&
        status.queueHighWater == 10);
  CHECK(health.recordSample(UINT32_MAX) == Status::InvalidTime);

  std::printf("PASS SuperJupiterDeviceHealth review: %u checks\n", checks);
}
