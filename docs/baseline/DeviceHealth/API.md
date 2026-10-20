# SuperJupiter DeviceHealth API

Include `SuperJupiterDeviceHealth.h`. All public types/functions are inside `superjupiter::device_health`.

## Behaviour and limits

Caller reports sampling and operational events; this core does not probe hardware or perform watchdog resets.
recordSample tracks the largest observed interval and count of intervals exceeding expectedIntervalMs+toleranceMs.
snapshot reports sample age and stale state against a caller clock with distances below 2^31 ms.
Failure/drop/restart counters saturate and expose saturation. Queue high-water is caller-observed, not an inferred queue capacity.
All counters are RAM only; persistence and aggregation belong to the application. No claim that an apparently healthy device is physically sound.

## Minimal complete usage

```cpp
#include <SuperJupiterDeviceHealth.h>
using namespace superjupiter::device_health;
int main() {
DeviceHealth health(100,20,1000);
health.recordSample(0);health.observeQueue(3);
Snapshot status=health.snapshot(100);
(void)status;
return 0;
}

```

## Public declarations

The [public header](https://github.com/Super-Jupiter/superjupiter-device-health/blob/v0.1.0/src/SuperJupiterDeviceHealth.h) is the complete API; constructors, result fields and status enums are visible there.
All time arguments are caller supplied. No method is thread-safe without caller synchronisation.
Non-success result fields must only be used where this contract explicitly defines them.
Memory is bounded by template capacities and local arrays; storage/network adapter implementations may have their own costs.


## 0.1.1 review clarifications

See [INTEROPERABILITY.md](INTEROPERABILITY.md) for units, timestamp horizons, numeric validity, payload constraints and storage synchronisation.

- Removed unused duplicated math helpers and unnecessary includes.
