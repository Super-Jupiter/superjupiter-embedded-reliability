// Consolidated public release baseline: SuperJupiterPowerBudget 0.1.1.
#if defined(SUPERJUPITER_POWER_BUDGET_H)
#error "Duplicate SuperJupiter module: use legacy OR consolidated headers for this component in one application"
#endif
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 SuperJupiter
#ifndef SUPERJUPITER_EMBEDDEDRELIABILITY_POWERBUDGET_H
#define SUPERJUPITER_EMBEDDEDRELIABILITY_POWERBUDGET_H
#if defined(__FAST_MATH__) ||                                                  \
    (defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__ > 0)
#error "SuperJupiter requires IEEE non-finite checks; disable fast-math"
#endif
#include <float.h>
#include <stddef.h>
#include <stdint.h>
#define SUPERJUPITER_POWER_BUDGET_VERSION "0.1.1"
namespace superjupiter {
namespace power_budget {

namespace detail {
inline bool finite(double value) {
  return value == value && value >= -DBL_MAX && value <= DBL_MAX;
}
} // namespace detail

enum class Status { Ok, NoLoad, InvalidInput, NumericError };
struct State {
  double currentMa;
  uint32_t durationMs;
};
struct Result {
  Status status;
  uint64_t cycleMs;
  double averageMa, mahPerDay, runtimeHours;
};
inline Result estimate(const State *states, size_t count, double capacityMah,
                       double usableFraction = 0.8, double backgroundMa = 0) {
  Result r = {Status::InvalidInput, 0, 0, 0, 0};
  if (!states || !count || !detail::finite(capacityMah) || capacityMah <= 0 ||
      !detail::finite(usableFraction) || usableFraction <= 0 ||
      usableFraction > 1 || !detail::finite(backgroundMa) || backgroundMa < 0)
    return r;
  for (size_t i = 0; i < count; ++i) {
    if (!detail::finite(states[i].currentMa) || states[i].currentMa < 0 ||
        r.cycleMs > UINT64_MAX - states[i].durationMs)
      return r;
    r.cycleMs += states[i].durationMs;
  }
  if (!r.cycleMs)
    return r;
  bool positiveLoad = backgroundMa > 0;
  for (size_t i = 0; i < count; ++i)
    if (states[i].durationMs && states[i].currentMa > 0)
      positiveLoad = true;
  r.averageMa = backgroundMa;
  for (size_t i = 0; i < count; ++i)
    r.averageMa +=
        states[i].currentMa * (static_cast<double>(states[i].durationMs) /
                               static_cast<double>(r.cycleMs));
  if (!detail::finite(r.averageMa)) {
    r.status = Status::NumericError;
    return r;
  }
  if (r.averageMa == 0) {
    r.status = positiveLoad ? Status::NumericError : Status::NoLoad;
    return r;
  }
  r.mahPerDay = r.averageMa * 24;
  const double ratio = capacityMah / r.averageMa;
  r.runtimeHours = detail::finite(ratio)
                       ? ratio * usableFraction
                       : (capacityMah * usableFraction) / r.averageMa;
  r.status = detail::finite(r.mahPerDay) && detail::finite(r.runtimeHours) &&
                     r.runtimeHours > 0
                 ? Status::Ok
                 : Status::NumericError;
  return r;
}
} // namespace power_budget
} // namespace superjupiter
#endif
