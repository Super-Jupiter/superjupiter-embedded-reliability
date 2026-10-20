namespace superjupiter {
struct Config {};
struct Result {};
namespace unrelated {
struct State {};
} // namespace unrelated
} // namespace superjupiter
#include <SuperJupiter/EmbeddedReliability/utilities/PowerBudget.h>
using namespace superjupiter::power_budget;
int main() {
  State profile[] = {{80, 100}, {0.1, 9900}};
  Result battery = estimate(profile, 2, 2000, 0.8);
  (void)battery;
  return 0;
}
