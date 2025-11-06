/**
 * Software License Agreement (proprietary)
 *
 * @copyright Copyright (c) 2025 Clearpath Robotics, Inc., All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, is not permitted without the express permission of Clearpath
 * Robotics.
 *
 * @author Roni Kreinin (rkreinin@clearpathrobotics.com)
 */

#ifndef INC_PROTONCPP_COMMON_HPP_
#define INC_PROTONCPP_COMMON_HPP_

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <stdint.h>

namespace proton {

enum TransportState {
  DISCONNECTED,
  CONNECTED,
  ERR
};

enum NodeState {
  UNCONFIGURED, // Node has not been configured yet
  INACTIVE,     // Node is configured but not active
  ACTIVE,       // Node is configured and active
  SHUTDOWN      // Node is shutdown due to error
};

enum Status {
  OK,    // No error
  ERROR, // Generic error
  NULL_PTR,
  INVALID_STATE,            // Attempting to call a function in an invalid state
  INVALID_STATE_TRANSITION, // Invalid node state transition
  CONNECTION_ERROR,         // Error connecting
  SERIALIZATION_ERROR,      // Error when serializing or deserializing bundle
  READ_ERROR,
  WRITE_ERROR,
  INVALID_HEADER, // Invalid header received over serial
  CRC16_ERROR,    // CRC's do not match
};

template <typename T> class SafeQueue {
public:
  // Producer: Push item
  void push(T item) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      queue_.push(std::move(item));
    }
    cv_.notify_one(); // Wake up consumer if waiting
  }

  // Consumer: Pop item (blocking)
  T pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !queue_.empty(); });
    T item = std::move(queue_.front());
    queue_.pop();
    return item;
  }

  // Consumer: Try pop (non-blocking)
  std::optional<T> try_pop() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) {
      return std::nullopt;
    }
    T item = std::move(queue_.front());
    queue_.pop();
    return item;
  }

  // Check if empty
  bool empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
  }

  // Get size
  size_t size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

private:
  std::queue<T> queue_;
  mutable std::mutex mutex_;
  std::condition_variable cv_;
};

} // namespace proton

#endif // INC_PROTONCPP_COMMON_HPP_
