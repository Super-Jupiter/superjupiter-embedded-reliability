# SuperJupiter Embedded Reliability

**Preview release 0.1.0. Hardware validation pending.**

Storage protocols, acknowledged RAM queues and diagnostics. Real persistence depends on a correctly implemented storage adapter and its sync contract. StoreForward does not provide networking or durable storage.

Original public MIT components by [SuperJupiter](https://www.superjupiter.net).
Independent C++11 package; no mandatory dependency on the other consolidated library, no service,
no automatic network traffic and no automatically started tasks. Select individual module headers.
The convenience header excludes testing/tools-only modules. Resource use depends on selected
components and capacities; no claim of zero flash or RAM cost is made.

## Modules

| Module | Include | Public baseline version |
| --- | --- | --- |
| RecordJournal | `SuperJupiter/EmbeddedReliability/RecordJournal.h` | 0.1.1 |
| StoreForward | `SuperJupiter/EmbeddedReliability/StoreForward.h` | 0.1.1 |
| ConfigStore | `SuperJupiter/EmbeddedReliability/ConfigStore.h` | 0.1.1 |
| DeviceHealth | `SuperJupiter/EmbeddedReliability/DeviceHealth.h` | 0.1.1 |
| PowerBudget | `SuperJupiter/EmbeddedReliability/utilities/PowerBudget.h` | 0.1.1 |

## Installation

Arduino: download the versioned ZIP from the [0.1.0 release](https://github.com/Super-Jupiter/superjupiter-embedded-reliability/releases/tag/v0.1.0) and use Sketch > Include Library > Add .ZIP Library.
Library Manager registration and index propagation are separate; check the official index before relying on IDE availability.

PlatformIO exact version:

```ini
lib_deps = superjupiter/SuperJupiterEmbeddedReliability@0.1.0
```

For manual installation or offline builds, use the versioned GitHub release assets.

```cmake
add_subdirectory(path/to/superjupiter-embedded-reliability)
target_link_libraries(your_app PRIVATE SuperJupiter::EmbeddedReliability)
```

After CMake installation: `find_package(SuperJupiterEmbeddedReliability 0.1 CONFIG REQUIRED)`.
Tests default off in add_subdirectory consumers. No global compiler flags are changed.

## Examples and limits

The examples directory contains module examples adapted from the exact public releases.
Examples use synthetic readings or RAM storage unless explicitly stated. Outcomes must be checked
before using results. Preserve raw data. No physical measurements, safety guarantees or field
durability are claimed. Component documentation is retained under docs/baseline; its versioned
references describe the source component, not a separately installed dependency of this bundle.

See [migration](docs/MIGRATION.md), [validation](docs/VALIDATION.md), and BASELINE.json for provenance.
Do not combine legacy and consolidated implementations of the same component in one application.
Public examples exclude customer information, proprietary products and private configurations.

## Support

Report reproducible issues through [GitHub issues](https://github.com/Super-Jupiter/superjupiter-embedded-reliability/issues). For private security reports contact info@superjupiter.net. Never include credentials or client data.

## Verify locally

```sh
python3 tools/verify.py
python3 tools/verify.py --test --compiler g++
```

Start with `examples/RecoverableRecords`. All hardware validation remains pending.
