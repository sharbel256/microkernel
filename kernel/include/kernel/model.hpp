#include <cstddef>
#include <cstdint>

namespace trading {

struct Message {
  uint32_t type;  // e.g., ORDERBOOK_DELTA, ORDER_REQUEST
  uint64_t sender_id;
  std::byte payload[128];
};

struct Plugin {
  using Handler = void (*)(const Message&);
  virtual void init() = 0;
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual void handle_message(const Message&) = 0;
  virtual ~Plugin() = default;
};

}  // namespace trading