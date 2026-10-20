#include <SuperJupiter/EmbeddedReliability/ConfigStore.h>
using namespace superjupiter::config_store;
int main() {
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

  Memory storage; // RAM illustration, replace with a durable adapter.
  ConfigStore<4> settings(storage, 1);
  uint8_t bytes[4] = {10, 0, 0, 0};
  uint8_t previous[4] = {};
  settings.load(previous);
  settings.save(bytes);
  return 0;
}
