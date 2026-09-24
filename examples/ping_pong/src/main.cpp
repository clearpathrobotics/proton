/*
 * Copyright 2026 Rockwell Automation Technologies, Inc., All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @author Tom Wallis (thomas.wallis@rockwellautomation.com)
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

#include <array>
#include <atomic>
#include <cerrno>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <format>
#include <iostream>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <thread>
#include <vector>

#include <proton/log.h>
#include <proton/node_manager.h>
#include <proton/registry.h>
#include <proton/transport.h>

#include <protoncpp/bundle_access.hpp>
#include <protoncpp/node_access.hpp>
#include <protoncpp/node_builder/generator.hpp>
#include <protoncpp/transport/core_udp4.hpp>

#include "target_connections.h"
#include "target_registry_ids.h"

extern proton_registry_t g_proton_registry;
extern proton_node_t g_target_node;

constexpr size_t BUFFER_SIZE = 512;
constexpr const char * CONFIG_PATH = PING_PONG_CONFIG_PATH;

std::atomic_bool g_running{true};

int make_udp_socket(uint32_t ip_addr_h, uint16_t port, sockaddr_in * addr)
{
  int fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
  if (fd < 0)
  {
    std::cerr << std::format("socket(): {}\n", std::strerror(errno));
    return -1;
  }

  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = htonl(ip_addr_h);
  addr->sin_port = htons(port);

  if (bind(fd, reinterpret_cast<const sockaddr *>(addr), sizeof(sockaddr_in)) < 0)
  {
    std::cerr << std::format("bind({}): {}\n", port, std::strerror(errno));
    close(fd);
    return -1;
  }

  return fd;
}

int make_periodic_timer(int period_ms)
{
  int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
  if (fd < 0)
  {
    std::cerr << std::format("timerfd_create(): {}\n", std::strerror(errno));
    return -1;
  }

  itimerspec spec{};
  spec.it_interval.tv_sec = period_ms / 1000;
  spec.it_interval.tv_nsec = (period_ms % 1000) * 1000000L;
  spec.it_value = spec.it_interval;

  if (timerfd_settime(fd, 0, &spec, nullptr) < 0)
  {
    std::cerr << std::format("timerfd_settime(): {}\n", std::strerror(errno));
    close(fd);
    return -1;
  }

  return fd;
}

/**
 * @brief Utility function to get the current uptime in ms.
 * Needed for node manager reads and writes
 */
uint64_t get_time_ms()
{
  struct timespec spec;
  clock_gettime(CLOCK_REALTIME, &spec);
  return ((int64_t)spec.tv_sec * 1000) + (spec.tv_nsec / 1000000);
}

std::string log_level_to_string(proton_Log_Level level)
{
  std::string level_str;
  switch (level)
  {
    case proton_Log_Level_LEVEL_TRACE:
      level_str = "TRACE";
      break;
    case proton_Log_Level_LEVEL_DEBUG:
      level_str = "DEBUG";
      break;
    case proton_Log_Level_LEVEL_INFO:
      level_str = "INFO";
      break;
    case proton_Log_Level_LEVEL_WARN:
      level_str = "WARN";
      break;
    case proton_Log_Level_LEVEL_ERROR:
      level_str = "ERROR";
      break;
    case proton_Log_Level_LEVEL_FATAL:
      level_str = "FATAL";
      break;
    default:
      level_str = "Unknown";
      break;
  }

  return level_str;
}

extern "C" void print_log(const proton_Log * log, void * arg)
{
  std::ignore = arg;

  std::string to_print = std::format(
    "({}) [{}] {}: {}", log->timestamp_ms, log_level_to_string(log->level), log->name, log->text);

  std::cout << to_print << '\n';
}

/**
 * @brief Called by proton_node_receive once a send_pong bundle has been decoded into the registry.
 * @note the registry mutex is already held here, so the signal accessors are safe to call.
 */
extern "C" void on_pong_received(
  uint32_t bundle_id, const uint32_t * signal_ids, size_t num_signals, void * arg)
{
  proton_registry_t * registry = static_cast<proton_registry_t *>(arg);

  for (size_t i = 0; i < num_signals; i++)
  {
    if (signal_ids[i] == PROTON_SIGNAL_PONG_ID)
    {
      int32_t pong_sig = 0;
      if (proton_signal_get_int32(registry, PROTON_SIGNAL_PONG_ID, &pong_sig) == PROTON_OK)
      {
        std::cout << std::format(
          "pinger: received bundle id {}, signal value {}\n", bundle_id, pong_sig);
      }
    }
  }
}

/**
 * This thread represents a resource-constrained device using the static signal registry
 * and node manager. The BSD socket is really just for example's sake.
 */
extern "C" void pinger_node_thread()
{
  int32_t ping_sig = 0;

  g_target_node.registry = &g_proton_registry;
  proton_node_set_log_receive(&g_target_node, print_log, nullptr);
  proton_registry_set_bundle_callback(
    &g_proton_registry, PROTON_BUNDLE_SEND_PONG_ID, on_pong_received, &g_proton_registry);

  sockaddr_in addr{};
  int sock_fd = make_udp_socket(
    PROTON_NODE_PINGER_ENDPOINT_0_TRANSPORT_IPHL, PROTON_NODE_PINGER_ENDPOINT_0_TRANSPORT_PORT,
    &addr);

  sockaddr_in ponger_addr{};
  ponger_addr.sin_family = AF_INET;
  ponger_addr.sin_addr.s_addr = htonl(PROTON_NODE_PONGER_ENDPOINT_0_TRANSPORT_IPHL);
  ponger_addr.sin_port = htons(PROTON_NODE_PONGER_ENDPOINT_0_TRANSPORT_PORT);

  const bundle_desc_t * send_ping_bundle =
    proton_registry_get_bundle(&g_proton_registry, PROTON_BUNDLE_SEND_PING_ID, nullptr);
  uint32_t ping_time_ms = send_ping_bundle->period_ms;
  int timer_fd = make_periodic_timer(ping_time_ms);

  if (sock_fd < 0 || timer_fd < 0)
  {
    if (sock_fd >= 0)
    {
      close(sock_fd);
    }
    if (timer_fd >= 0)
    {
      close(timer_fd);
    }
    g_running = false;
    return;
  }

  std::array<uint8_t, BUFFER_SIZE> rx_buffer{};
  std::array<uint8_t, BUFFER_SIZE> tx_buffer{};
  pollfd fds[2] = {
    {sock_fd, POLLIN, 0},
    {timer_fd, POLLIN, 0},
  };

  while (g_running)
  {
    // For the sake of this demo we are going to update the ping signal twice per bundle update
    if (poll(fds, 2, ping_time_ms / 2) < 0)
    {
      if (errno == EINTR)
      {
        continue;
      }
      std::cerr << std::format("pinger poll(): {}\n", std::strerror(errno));
      break;
    }

    uint64_t current_time_ms = get_time_ms();

    if (fds[0].revents & POLLIN)
    {
      sockaddr_in src{};
      socklen_t src_len = sizeof(src);
      ssize_t received = recvfrom(
        sock_fd, rx_buffer.data(), rx_buffer.size(), 0, reinterpret_cast<sockaddr *>(&src),
        &src_len);

      if (received > 0)
      {
        // Strip the udp4 header off before handing the payload to the node manager
        proton_udp4_header_t rx_header;
        if (
          proton_udp4_check_payload(
            rx_buffer.data(), static_cast<uint16_t>(received), &rx_header) == PROTON_OK &&
          rx_header.version == UDP4_VERSION_2)
        {
          proton_status_e rx_status = proton_node_receive(
            &g_target_node, rx_buffer.data() + sizeof(rx_header),
            static_cast<size_t>(received) - sizeof(rx_header));

          if (rx_status == PROTON_OK)
          {
            std::cout << std::format("pinger: received data from peer\n");
          }
          else
          {
            std::cerr << std::format(
              "pinger: error receiving data from peer {}\n", proton_status_to_string(rx_status));
          }
        }
      }
    }

    if (fds[1].revents & POLLIN)
    {
      uint64_t expirations = 0;
      if (read(timer_fd, &expirations, sizeof(expirations)) == sizeof(expirations))
      {
        // Update the ping signal and write to the registry
        ping_sig += 1;
        proton_signal_set_int32(&g_proton_registry, PROTON_SIGNAL_PING_ID, ping_sig);
      }
    }

    std::size_t out_len = 0;
    std::size_t num_selected_peers = 0;
    proton_endpoint_t dest_peers[1];

    // Create a header for the tx buffer
    proton_udp4_header_t tx_header;
    proton_udp4_fill_header(&tx_header, PROTON_NODE_PINGER_ID, 0);
    std::memcpy(tx_buffer.data(), &tx_header, sizeof(tx_header));

    // Spin the node manager, see if there is a new bundle to transmit
    // Pass a pointer after the header, and set the size accordingly
    if (
      proton_node_update(
        &g_target_node, current_time_ms, tx_buffer.data() + sizeof(tx_header),
        tx_buffer.size() - sizeof(tx_header), &out_len, dest_peers, 1,
        &num_selected_peers) == PROTON_OK)
    {
      // Realistically the user will maintain a table of sock_fd's matched to
      // peers for UDP4, but in this case there's only one so it's pretty trivial
      if (num_selected_peers > 0)
      {
        ssize_t sent = sendto(
          sock_fd, tx_buffer.data(), out_len + sizeof(tx_header), 0,
          reinterpret_cast<const sockaddr *>(&ponger_addr), sizeof(ponger_addr));

        if (sent < 0)
        {
          std::cerr << std::format("pinger sendto(): {}\n", std::strerror(errno));
        }
      }
    }
  }

  close(timer_fd);
  close(sock_fd);
}

/**
 * This thread represents a full-featured peer with access to proton_cpp's node_builder feature
 */
extern "C" void ponger_node_thread()
{
  using namespace proton::node_builder;

  int32_t pong_sig = 0;

  std::unique_ptr<GeneratedNode> generated;
  try
  {
    Config config = Config::from_yaml(CONFIG_PATH);
    generated = std::make_unique<GeneratedNode>(
      filter_for_target(config, PROTON_NODE_PONGER_NAME), PROTON_NODE_PONGER_NAME);
  }
  catch (const std::exception & e)
  {
    std::cerr << std::format("ponger node_builder: {}\n", e.what());
    g_running = false;
    return;
  }

  proton::NodeAccess node(generated->node());
  node.set_log_receive(print_log, nullptr);

  // proton_node_receive holds the registry lock for the duration of this callback,
  // so only the (lock-free) signal accessors are safe to call from here.
  node.on_bundle_update(
    PROTON_BUNDLE_SEND_PING_ID,
    [node](uint32_t bundle_id, const uint32_t * signal_ids, size_t num_signals) mutable
    {
      std::optional<proton::BundleAccess> bundle = node[bundle_id];
      std::span<const uint32_t> signals(signal_ids, num_signals);
      if (!bundle.has_value())
      {
        std::cerr << std::format("Bundle {} not found\n", bundle_id);
      }
      else
      {
        for (const auto sig_id : signals)
        {
          if (sig_id == PROTON_SIGNAL_PING_ID)
          {
            int32_t ping_sig = 0;
            if (node.signals().get(PROTON_SIGNAL_PING_ID, ping_sig) == PROTON_OK)
            {
              std::cout << std::format(
                "ponger: received bundle id {}, ping value {}\n", bundle_id, ping_sig);
            }
          }
        }
      }
    });

  sockaddr_in addr{};
  int sock_fd = make_udp_socket(
    PROTON_NODE_PONGER_ENDPOINT_0_TRANSPORT_IPHL, PROTON_NODE_PONGER_ENDPOINT_0_TRANSPORT_PORT,
    &addr);

  sockaddr_in pinger_addr{};
  pinger_addr.sin_family = AF_INET;
  pinger_addr.sin_addr.s_addr = htonl(PROTON_NODE_PINGER_ENDPOINT_0_TRANSPORT_IPHL);
  pinger_addr.sin_port = htons(PROTON_NODE_PINGER_ENDPOINT_0_TRANSPORT_PORT);

  uint32_t pong_time_ms = node.bundle(PROTON_BUNDLE_SEND_PONG_ID).descriptor()->period_ms;
  int timer_fd = make_periodic_timer(pong_time_ms);

  if (sock_fd < 0 || timer_fd < 0)
  {
    if (sock_fd >= 0)
    {
      close(sock_fd);
    }
    if (timer_fd >= 0)
    {
      close(timer_fd);
    }
    g_running = false;
    return;
  }

  std::array<uint8_t, BUFFER_SIZE> rx_buffer{};
  std::array<uint8_t, BUFFER_SIZE> tx_buffer{};
  pollfd fds[2] = {
    {sock_fd, POLLIN, 0},
    {timer_fd, POLLIN, 0},
  };

  while (g_running)
  {
    if (poll(fds, 2, pong_time_ms) < 0)
    {
      if (errno == EINTR)
      {
        continue;
      }
      std::cerr << std::format("ponger poll(): {}\n", std::strerror(errno));
      break;
    }

    uint64_t current_time_ms = get_time_ms();

    if (fds[0].revents & POLLIN)
    {
      sockaddr_in src{};
      socklen_t src_len = sizeof(src);
      ssize_t received = recvfrom(
        sock_fd, rx_buffer.data(), rx_buffer.size(), 0, reinterpret_cast<sockaddr *>(&src),
        &src_len);

      if (received > 0)
      {
        std::span<const uint8_t> datagram{rx_buffer.data(), static_cast<size_t>(received)};

        proton::transport::udp4::Header rx_header{};
        if (
          proton::transport::udp4::check_payload(datagram, rx_header) == PROTON_OK &&
          rx_header.version == proton::transport::udp4::CURRENT_VERSION)
        {
          // The send_ping callback registered above updates the pong signal from here
          node.receive(datagram.subspan(sizeof(rx_header)));
        }
      }
    }

    if (fds[1].revents & POLLIN)
    {
      uint64_t expirations = 0;
      std::ignore = read(timer_fd, &expirations, sizeof(expirations));
    }

    // Update pong signal
    node.signals().set(PROTON_SIGNAL_PONG_ID, pong_sig++);

    size_t out_len = 0;
    size_t num_selected_peers = 0;
    std::array<proton::NodeAccess::Endpoint, 1> dest_peers{};

    proton::transport::udp4::Header tx_header{};
    proton::transport::udp4::fill_header(tx_header, PROTON_NODE_PONGER_ID, 0);
    std::memcpy(tx_buffer.data(), &tx_header, sizeof(tx_header));

    std::span<uint8_t> payload{
      tx_buffer.data() + sizeof(tx_header), tx_buffer.size() - sizeof(tx_header)};

    if (
      node.update(current_time_ms, payload, out_len, dest_peers, num_selected_peers) == PROTON_OK &&
      num_selected_peers > 0)
    {
      ssize_t sent = sendto(
        sock_fd, tx_buffer.data(), out_len + sizeof(tx_header), 0,
        reinterpret_cast<const sockaddr *>(&pinger_addr), sizeof(pinger_addr));

      if (sent < 0)
      {
        std::cerr << std::format("ponger sendto(): {}\n", std::strerror(errno));
      }
    }
  }

  close(timer_fd);
  close(sock_fd);
}

int main()
{
  std::signal(SIGINT, [](int) { g_running = false; });

  std::thread pinger(pinger_node_thread);
  std::thread ponger(ponger_node_thread);

  pinger.join();
  ponger.join();

  return 0;
}
