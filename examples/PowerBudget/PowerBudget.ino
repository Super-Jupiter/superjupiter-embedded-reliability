#include <Arduino.h>
#include <SuperJupiter/EmbeddedReliability/utilities/PowerBudget.h>
using namespace superjupiter::power_budget;
void setup() {
  Serial.begin(115200);
  State profile[] = {{80, 100}, {0.1, 9900}};
  Result battery = estimate(profile, 2, 2000, 0.8);
  (void)battery;
  Serial.println("SuperJupiter PowerBudget example ran. Hardware validation pending.");
}
void loop() {}
