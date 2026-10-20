#include <SuperJupiter/EmbeddedReliability/DeviceHealth.h>
using namespace superjupiter::device_health;
int main() {
  DeviceHealth health(100, 20, 1000);
  health.recordSample(0);
  health.observeQueue(3);
  Snapshot status = health.snapshot(100);
  (void)status;
  return 0;
}
