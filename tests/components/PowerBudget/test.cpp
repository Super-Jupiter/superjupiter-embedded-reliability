#include <SuperJupiter/EmbeddedReliability/utilities/PowerBudget.h>
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
using namespace superjupiter::power_budget;
void *operator new(std::size_t) { std::abort(); }
void *operator new[](std::size_t) { std::abort(); }
void operator delete(void *p) noexcept { std::free(p); }
void operator delete[](void *p) noexcept { std::free(p); }
int main() {
  (void)near;
  State profile[] = {{100, 1000}, {0, 9000}};
  Result r = estimate(profile, 2, 1000, 0.8);
  CHECK(r.status == Status::Ok && r.averageMa == 10 && r.mahPerDay == 240 && r.runtimeHours == 80 &&
        r.cycleMs == 10000);
  CHECK(estimate(profile, 2, 1000, 0.8, 10).runtimeHours == 40);
  State off[] = {{0, 1000}};
  CHECK(estimate(off, 1, 1000).status == Status::NoLoad);
  State bad[] = {{NAN, 1}};
  CHECK(estimate(bad, 1, 1000).status == Status::InvalidInput);
  CHECK(estimate(0, 0, 1000).status == Status::InvalidInput);
  CHECK(estimate(profile, 2, 1000, 1.1).status == Status::InvalidInput);

  for (unsigned duration = 1; duration < 1000; ++duration) {
    State load[] = {{10, duration}, {30, duration}};
    Result v = estimate(load, 2, 1000, 1);
    CHECK(near(v.averageMa, 20));
    CHECK(near(v.runtimeHours, 50));
  }
  State zero[] = {{1, 0}};
  CHECK(estimate(zero, 1, 1).status == Status::InvalidInput);

  std::printf("PASS SuperJupiterPowerBudget: %lu checks\n", checks);
}
