#include <SuperJupiter/EmbeddedReliability/ConfigStore.h>
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
using namespace superjupiter::config_store;
void *operator new(std::size_t) { std::abort(); }
void *operator new[](std::size_t) { std::abort(); }
void operator delete(void *p) noexcept { std::free(p); }
void operator delete[](void *p) noexcept { std::free(p); }
int main() {
  (void)near;
  class Memory : public Storage {
  public:
    uint8_t bytes[256];
    int budget;
    bool failSync;
    int syncBudget = -1;
    bool failRead = false;
    Memory() : budget(-1), failSync(false) {
      for (size_t i = 0; i < sizeof(bytes); ++i)
        bytes[i] = 255;
    }
    bool read(size_t o, uint8_t *p, size_t n) {
      if (failRead)
        return false;
      if (o > sizeof(bytes) || n > sizeof(bytes) - o)
        return false;
      for (size_t i = 0; i < n; ++i)
        p[i] = bytes[o + i];
      return true;
    }
    bool write(size_t o, const uint8_t *p, size_t n) {
      if (o > sizeof(bytes) || n > sizeof(bytes) - o)
        return false;
      for (size_t i = 0; i < n; ++i) {
        if (budget == 0)
          return false;
        bytes[o + i] = p[i];
        if (budget > 0)
          --budget;
      }
      return true;
    }
    bool sync() {
      if (syncBudget == 0)
        return false;
      if (syncBudget > 0)
        --syncBudget;
      return !failSync;
    }
  };

  Memory m;
  uint8_t first[4] = {1, 2, 3, 4}, second[4] = {9, 8, 7, 6}, out[4] = {};
  ConfigStore<4> c(m, 7);
  CHECK(c.save(first) == Status::NotLoaded);
  CHECK(c.load(out) == Status::Empty);
  CHECK(c.save(first) == Status::Ok);
  CHECK(c.revision() == 1);
  CHECK(c.save(second) == Status::Ok);
  ConfigStore<4> reboot(m, 7);
  CHECK(reboot.load(out) == Status::Ok && out[0] == 9 && reboot.revision() == 2);
  m.bytes[29 + 20] ^= 1;
  CHECK(reboot.load(out) == Status::Recovered && out[0] == 1);
  ConfigStore<4> wrong(m, 8);
  CHECK(wrong.load(out) == Status::SchemaMismatch);
  CHECK(wrong.save(first) == Status::NotLoaded);
  for (int cut = 0; cut <= 30; ++cut) {
    Memory mem;
    ConfigStore<4> store(mem, 1);
    store.load(out);
    CHECK(store.save(first) == Status::Ok);
    mem.budget = cut;
    store.save(second);
    mem.budget = -1;
    ConfigStore<4> recovered(mem, 1);
    CHECK(recovered.load(out) == Status::Ok);
    CHECK(out[0] == 1 || out[0] == 9);
  }

  for (int failure = 0; failure < 3; ++failure) {
    Memory mem;
    ConfigStore<4> store(mem, 1);
    store.load(out);
    store.save(first);
    mem.syncBudget = failure;
    CHECK(store.save(second) == Status::IoError);
    CHECK(store.save(second) == Status::NotLoaded);
    mem.syncBudget = -1;
    ConfigStore<4> restored(mem, 1);
    CHECK(restored.load(out) == Status::Ok);
    CHECK(out[0] == 1 || out[0] == 9);
  }
  Memory unreadable;
  unreadable.failRead = true;
  ConfigStore<4> unavailable(unreadable, 1);
  CHECK(unavailable.load(out) == Status::IoError);
  Memory corrupt;
  ConfigStore<4> initial(corrupt, 1);
  initial.load(out);
  initial.save(first);
  corrupt.bytes[20] ^= 1;
  ConfigStore<4> damaged(corrupt, 1);
  CHECK(damaged.load(out) == Status::Corrupt);

  std::printf("PASS SuperJupiterConfigStore: %lu checks\n", checks);
}
