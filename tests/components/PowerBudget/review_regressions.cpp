// SPDX-License-Identifier: MIT
#include <SuperJupiter/EmbeddedReliability/utilities/PowerBudget.h>
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
using namespace superjupiter::power_budget;

int main() {
  (void)close;

  const double tiny = std::numeric_limits<double>::denorm_min();
  State load[] = {{tiny, 1}};
  Result half = estimate(load, 1, tiny, 0.5);
  CHECK(half.status == Status::Ok && half.runtimeHours == 0.5);
  State diluted[] = {{tiny, 1}, {0, 1}};
  CHECK(estimate(diluted, 2, 1).status == Status::NumericError);
  State off[] = {{0, 1}};
  CHECK(estimate(off, 1, 1000).status == Status::NoLoad);

  std::printf("PASS SuperJupiterPowerBudget review: %u checks\n", checks);
}
