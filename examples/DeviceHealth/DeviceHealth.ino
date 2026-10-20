#include <Arduino.h>
#include <SuperJupiter/EmbeddedReliability/DeviceHealth.h>
using namespace superjupiter::device_health;
void setup() {
  Serial.begin(115200);
  DeviceHealth health(100, 20, 1000);
  health.recordSample(0);
  health.observeQueue(3);
  Snapshot status = health.snapshot(100);
  (void)status;
  Serial.println("SuperJupiter DeviceHealth example ran. Hardware validation pending.");
}
void loop() {}
