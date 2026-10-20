# SuperJupiter ConfigStore API

Include `SuperJupiterConfigStore.h`. All public types/functions are inside `superjupiter::config_store`.

## Behaviour and limits

Caller supplies byte-rewritable storage of requiredBytes(), initially 0xff. Adapter sync must make prior writes durable.
Two slots contain fixed-size opaque bytes, schema ID, monotonic generation, CRC32 and a final commit marker.
load selects the newest valid slot, falls back from a damaged slot and reports Recovered; schema mismatch is explicit and blocks saves.
Application performs any schema migration itself in a separately controlled process. No implicit reinterpretation of old bytes.
Any failed save locks the instance until load reconciles it; a failed final sync may have committed, so never blindly retry.
No encryption, access control, wear levelling, raw-flash erase support or cross-task locking. Hardware power-loss validation pending.

## Minimal complete usage

```cpp
#include <SuperJupiterConfigStore.h>
using namespace superjupiter::config_store;
int main() {
class Memory: public Storage {
public:
 uint8_t bytes[256];int budget;bool failSync;
 Memory():budget(-1),failSync(false) {for(size_t i=0;i<sizeof(bytes);++i) bytes[i]=255;}
 bool read(size_t o,uint8_t* p,size_t n) {if(o>sizeof(bytes)||n>sizeof(bytes)-o) return false;for(size_t i=0;i<n;++i) p[i]=bytes[o+i];return true;}
 bool write(size_t o,const uint8_t* p,size_t n) {if(o>sizeof(bytes)||n>sizeof(bytes)-o) return false;for(size_t i=0;i<n;++i) {if(budget==0) return false;bytes[o+i]=p[i];if(budget>0) --budget;}return true;}
 bool sync() {return !failSync;}
};

Memory storage; // RAM illustration, replace with a durable adapter.
ConfigStore<4> settings(storage,1);
uint8_t bytes[4]={10,0,0,0};uint8_t previous[4]={};
settings.load(previous);settings.save(bytes);
return 0;
}

```

## Public declarations

The [public header](https://github.com/Super-Jupiter/superjupiter-config-store/blob/v0.1.0/src/SuperJupiterConfigStore.h) is the complete API; constructors, result fields and status enums are visible there.
All time arguments are caller supplied. No method is thread-safe without caller synchronisation.
Non-success result fields must only be used where this contract explicitly defines them.
Memory is bounded by template capacities and local arrays; storage/network adapter implementations may have their own costs.


## 0.1.1 review clarifications

See [INTEROPERABILITY.md](INTEROPERABILITY.md) for units, timestamp horizons, numeric validity, payload constraints and storage synchronisation.

- Removed unused duplicated math helpers and unnecessary includes.
- Sequential stale configuration writers cannot replace newer generations; no wire-format change.

StaleState rejects a cached handle after another sequential writer committed newer data. Reopen with recover/load; this preflight is not a substitute for external writer exclusion. The version-1 storage byte format is unchanged.
