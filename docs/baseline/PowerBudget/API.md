# SuperJupiter PowerBudget API

Include `SuperJupiterPowerBudget.h`. All public types/functions are inside `superjupiter::power_budget`.

## Behaviour and limits

Profile entries specify battery-side current in mA and durationMs for mutually exclusive operating states.
The supplied profile is one repeating cycle; average current is duration-weighted, with an optional constant background load.
usableFraction includes caller-estimated capacity derating and reserve. RuntimeHours=capacityMah*usableFraction/averageMa.
All nonzero-duration states contribute. Zero load reports NoLoad rather than a finite lifetime.
No battery chemistry, temperature, ageing, regulator or self-discharge model is inferred. Estimates require measured profiles; hardware validation pending.

## Minimal complete usage

```cpp
#include <SuperJupiterPowerBudget.h>
using namespace superjupiter::power_budget;
int main() {
State profile[]={{80,100},{0.1,9900}};
Result battery=estimate(profile,2,2000,0.8);
(void)battery;
return 0;
}

```

## Public declarations

The [public header](https://github.com/Super-Jupiter/superjupiter-power-budget/blob/v0.1.0/src/SuperJupiterPowerBudget.h) is the complete API; constructors, result fields and status enums are visible there.
All time arguments are caller supplied. No method is thread-safe without caller synchronisation.
Non-success result fields must only be used where this contract explicitly defines them.
Memory is bounded by template capacities and local arrays; storage/network adapter implementations may have their own costs.


## 0.1.1 review clarifications

See [INTEROPERABILITY.md](INTEROPERABILITY.md) for units, timestamp horizons, numeric validity, payload constraints and storage synchronisation.

- Removed unused duplicated math helpers and unnecessary includes.
- Positive subnormal loads cannot be reported as no load; avoid runtime underflow when final result is representable.
