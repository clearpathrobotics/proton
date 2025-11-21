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

enum class TransportState {
  DISCONNECTED,
  CONNECTED,
  ERROR
};

inline std::ostream& operator<<(std::ostream& os, TransportState state) {
  switch(state)
  {
    case TransportState::DISCONNECTED: return os << "Disconnected";
    case TransportState::CONNECTED: return os << "Connected";
    case TransportState::ERROR: return os << "Error";
    default: return os << "UNKNOWN";
  }
}

enum class NodeState {
  UNCONFIGURED, // Node has not been configured yet
  INACTIVE,     // Node is configured but not active
  ACTIVE        // Node is configured and active
};

inline std::ostream& operator<<(std::ostream& os, NodeState state) {
  switch(state)
  {
    case NodeState::UNCONFIGURED: return os << "Unconfigured";
    case NodeState::INACTIVE: return os << "Inactive";
    case NodeState::ACTIVE: return os << "Active";
    default: return os << "UNKNOWN";
  }
}

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
  INSUFFICIENT_BUFFER, // Buffer is too small to fit required data
};

inline std::ostream& operator<<(std::ostream& os, Status state) {
  switch(state)
  {
    case Status::OK: return os << "OK";
    case Status::ERROR: return os << "Error";
    case Status::NULL_PTR: return os << "Null Pointer";
    case Status::INVALID_STATE: return os << "Invalid State";
    case Status::INVALID_STATE_TRANSITION: return os << "Invalid State Transition";
    case Status::CONNECTION_ERROR: return os << "Connection Error";
    case Status::SERIALIZATION_ERROR: return os << "Serialization Error";
    case Status::READ_ERROR: return os << "Read Error";
    case Status::WRITE_ERROR: return os << "Write Error";
    case Status::INVALID_HEADER: return os << "Invalid Header Error";
    case Status::CRC16_ERROR: return os << "CRC16 Error";
    case Status::INSUFFICIENT_BUFFER: return os << "Insufficient Buffer";
    default: return os << "UNKNOWN";
  }
}

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
