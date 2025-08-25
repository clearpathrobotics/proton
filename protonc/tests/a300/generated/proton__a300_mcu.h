
/**
 * Software License Agreement (proprietary)
 *
 * @copyright Copyright (c) 2025 Clearpath Robotics, Inc., All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, is not permitted without the
 * express permission of Clearpath Robotics.
 *
 * THIS FILE WAS AUTOMATICALLY GENERATED. DO NOT MODIFY.
 */

#ifndef PROTONC__PROTON__A300_MCU_H
#define PROTONC__PROTON__A300_MCU_H

#include "stdint.h"
#include "stdbool.h"
#include "proton.h"

// Node Info

#define PROTON_NODE__PC__NAME "pc"
#define PROTON_NODE__PC__IP 0x7f000001
#define PROTON_NODE__PC__PORT 11417
#define PROTON_NODE__MCU__NAME "mcu"
#define PROTON_NODE__MCU__IP 0x7f000001
#define PROTON_NODE__MCU__PORT 11416

// Bundle IDs

typedef enum PROTON_BUNDLE {
  PROTON_BUNDLE__LOGGER = 0x100,
  PROTON_BUNDLE__STATUS = 0x101,
  PROTON_BUNDLE__POWER = 0x102,
  PROTON_BUNDLE__EMERGENCY_STOP = 0x103,
  PROTON_BUNDLE__TEMPERATURE = 0x104,
  PROTON_BUNDLE__STOP_STATUS = 0x105,
  PROTON_BUNDLE__PINOUT_STATE = 0x106,
  PROTON_BUNDLE__ALERTS = 0x107,
  PROTON_BUNDLE__CMD_FANS = 0x200,
  PROTON_BUNDLE__DISPLAY_STATUS = 0x201,
  PROTON_BUNDLE__CMD_LIGHTS = 0x202,
  PROTON_BUNDLE__BATTERY = 0x203,
  PROTON_BUNDLE__PINOUT_COMMAND = 0x204,
  PROTON_BUNDLE__CMD_SHUTDOWN = 0x300,
  PROTON_BUNDLE__CLEAR_NEEDS_RESET = 0x301,
} PROTON_BUNDLE_e;

// Signal Enums

typedef enum PROTON_SIGNALS__logger {
  PROTON_SIGNALS__LOGGER__LEVEL,
  PROTON_SIGNALS__LOGGER__NAME,
  PROTON_SIGNALS__LOGGER__MSG,
  PROTON_SIGNALS__LOGGER__FILE,
  PROTON_SIGNALS__LOGGER__FUNCTION,
  PROTON_SIGNALS__LOGGER__LINE,
  PROTON_SIGNALS__LOGGER_COUNT
} PROTON_SIGNALS__logger_e;

typedef enum PROTON_SIGNALS__status {
  PROTON_SIGNALS__STATUS__HARDWARE_ID,
  PROTON_SIGNALS__STATUS__FIRMWARE_VERSION,
  PROTON_SIGNALS__STATUS__MCU_UPTIME_S,
  PROTON_SIGNALS__STATUS__MCU_UPTIME_NS,
  PROTON_SIGNALS__STATUS__CONNECTION_UPTIME_S,
  PROTON_SIGNALS__STATUS__CONNECTION_UPTIME_NS,
  PROTON_SIGNALS__STATUS_COUNT
} PROTON_SIGNALS__status_e;

typedef enum PROTON_SIGNALS__power {
  PROTON_SIGNALS__POWER__MEASURED_VOLTAGES,
  PROTON_SIGNALS__POWER__MEASURED_CURRENTS,
  PROTON_SIGNALS__POWER_COUNT
} PROTON_SIGNALS__power_e;

typedef enum PROTON_SIGNALS__emergency_stop {
  PROTON_SIGNALS__EMERGENCY_STOP__STOPPED,
  PROTON_SIGNALS__EMERGENCY_STOP_COUNT
} PROTON_SIGNALS__emergency_stop_e;

typedef enum PROTON_SIGNALS__temperature {
  PROTON_SIGNALS__TEMPERATURE__TEMPERATURES,
  PROTON_SIGNALS__TEMPERATURE_COUNT
} PROTON_SIGNALS__temperature_e;

typedef enum PROTON_SIGNALS__stop_status {
  PROTON_SIGNALS__STOP_STATUS__NEEDS_RESET,
  PROTON_SIGNALS__STOP_STATUS_COUNT
} PROTON_SIGNALS__stop_status_e;

typedef enum PROTON_SIGNALS__pinout_state {
  PROTON_SIGNALS__PINOUT_STATE__RAILS,
  PROTON_SIGNALS__PINOUT_STATE__INPUTS,
  PROTON_SIGNALS__PINOUT_STATE__OUTPUTS,
  PROTON_SIGNALS__PINOUT_STATE__OUTPUT_PERIODS,
  PROTON_SIGNALS__PINOUT_STATE_COUNT
} PROTON_SIGNALS__pinout_state_e;

typedef enum PROTON_SIGNALS__alerts {
  PROTON_SIGNALS__ALERTS__ALERT_STRING,
  PROTON_SIGNALS__ALERTS_COUNT
} PROTON_SIGNALS__alerts_e;

typedef enum PROTON_SIGNALS__cmd_fans {
  PROTON_SIGNALS__CMD_FANS__FAN_SPEEDS,
  PROTON_SIGNALS__CMD_FANS_COUNT
} PROTON_SIGNALS__cmd_fans_e;

typedef enum PROTON_SIGNALS__display_status {
  PROTON_SIGNALS__DISPLAY_STATUS__STRING_1,
  PROTON_SIGNALS__DISPLAY_STATUS__STRING_2,
  PROTON_SIGNALS__DISPLAY_STATUS_COUNT
} PROTON_SIGNALS__display_status_e;

typedef enum PROTON_SIGNALS__cmd_lights {
  PROTON_SIGNALS__CMD_LIGHTS__FRONT_LEFT_LIGHT,
  PROTON_SIGNALS__CMD_LIGHTS__FRONT_RIGHT_LIGHT,
  PROTON_SIGNALS__CMD_LIGHTS__REAR_LEFT_LIGHT,
  PROTON_SIGNALS__CMD_LIGHTS__REAR_RIGHT_LIGHT,
  PROTON_SIGNALS__CMD_LIGHTS_COUNT
} PROTON_SIGNALS__cmd_lights_e;

typedef enum PROTON_SIGNALS__battery {
  PROTON_SIGNALS__BATTERY__PERCENTAGE,
  PROTON_SIGNALS__BATTERY_COUNT
} PROTON_SIGNALS__battery_e;

typedef enum PROTON_SIGNALS__pinout_command {
  PROTON_SIGNALS__PINOUT_COMMAND__RAILS,
  PROTON_SIGNALS__PINOUT_COMMAND__OUTPUTS,
  PROTON_SIGNALS__PINOUT_COMMAND_COUNT
} PROTON_SIGNALS__pinout_command_e;

typedef enum PROTON_SIGNALS__cmd_shutdown {
  PROTON_SIGNALS__CMD_SHUTDOWN_COUNT
} PROTON_SIGNALS__cmd_shutdown_e;

typedef enum PROTON_SIGNALS__clear_needs_reset {
  PROTON_SIGNALS__CLEAR_NEEDS_RESET_COUNT
} PROTON_SIGNALS__clear_needs_reset_e;


// Constant definitions

#define PROTON_SIGNALS__LOGGER__NAME__CAPACITY 64
#define PROTON_SIGNALS__LOGGER__MSG__CAPACITY 64
#define PROTON_SIGNALS__LOGGER__FILE__CAPACITY 64
#define PROTON_SIGNALS__LOGGER__FUNCTION__CAPACITY 64

#define PROTON_SIGNALS__STATUS__HARDWARE_ID__CAPACITY 10
#define PROTON_SIGNALS__STATUS__FIRMWARE_VERSION__CAPACITY 10

#define PROTON_SIGNALS__POWER__MEASURED_VOLTAGES__LENGTH 13
#define PROTON_SIGNALS__POWER__MEASURED_CURRENTS__LENGTH 13

#define PROTON_SIGNALS__TEMPERATURE__TEMPERATURES__LENGTH 18

#define PROTON_SIGNALS__PINOUT_STATE__RAILS__LENGTH 1
#define PROTON_SIGNALS__PINOUT_STATE__INPUTS__LENGTH 7
#define PROTON_SIGNALS__PINOUT_STATE__OUTPUTS__LENGTH 7
#define PROTON_SIGNALS__PINOUT_STATE__OUTPUT_PERIODS__LENGTH 7

#define PROTON_SIGNALS__ALERTS__ALERT_STRING__CAPACITY 86

#define PROTON_SIGNALS__CMD_FANS__FAN_SPEEDS__CAPACITY 8

#define PROTON_SIGNALS__DISPLAY_STATUS__STRING_1__CAPACITY 50
#define PROTON_SIGNALS__DISPLAY_STATUS__STRING_2__CAPACITY 50

#define PROTON_SIGNALS__CMD_LIGHTS__FRONT_LEFT_LIGHT__CAPACITY 3
#define PROTON_SIGNALS__CMD_LIGHTS__FRONT_RIGHT_LIGHT__CAPACITY 3
#define PROTON_SIGNALS__CMD_LIGHTS__REAR_LEFT_LIGHT__CAPACITY 3
#define PROTON_SIGNALS__CMD_LIGHTS__REAR_RIGHT_LIGHT__CAPACITY 3

#define PROTON_SIGNALS__PINOUT_COMMAND__RAILS__LENGTH 1
#define PROTON_SIGNALS__PINOUT_COMMAND__OUTPUTS__LENGTH 7

// Bundle Structure Definitions

typedef struct PROTON_BUNDLE__logger {
  uint32_t level;
  char name[PROTON_SIGNALS__LOGGER__NAME__CAPACITY];
  char msg[PROTON_SIGNALS__LOGGER__MSG__CAPACITY];
  char file[PROTON_SIGNALS__LOGGER__FILE__CAPACITY];
  char function[PROTON_SIGNALS__LOGGER__FUNCTION__CAPACITY];
  uint32_t line;
} PROTON_BUNDLE__logger_t;

typedef struct PROTON_BUNDLE__status {
  char hardware_id[PROTON_SIGNALS__STATUS__HARDWARE_ID__CAPACITY];
  char firmware_version[PROTON_SIGNALS__STATUS__FIRMWARE_VERSION__CAPACITY];
  uint32_t mcu_uptime_s;
  uint32_t mcu_uptime_ns;
  uint32_t connection_uptime_s;
  uint32_t connection_uptime_ns;
} PROTON_BUNDLE__status_t;

typedef struct PROTON_BUNDLE__power {
  float measured_voltages[PROTON_SIGNALS__POWER__MEASURED_VOLTAGES__LENGTH];
  float measured_currents[PROTON_SIGNALS__POWER__MEASURED_CURRENTS__LENGTH];
} PROTON_BUNDLE__power_t;

typedef struct PROTON_BUNDLE__emergency_stop {
  bool stopped;
} PROTON_BUNDLE__emergency_stop_t;

typedef struct PROTON_BUNDLE__temperature {
  float temperatures[PROTON_SIGNALS__TEMPERATURE__TEMPERATURES__LENGTH];
} PROTON_BUNDLE__temperature_t;

typedef struct PROTON_BUNDLE__stop_status {
  bool needs_reset;
} PROTON_BUNDLE__stop_status_t;

typedef struct PROTON_BUNDLE__pinout_state {
  bool rails[PROTON_SIGNALS__PINOUT_STATE__RAILS__LENGTH];
  bool inputs[PROTON_SIGNALS__PINOUT_STATE__INPUTS__LENGTH];
  bool outputs[PROTON_SIGNALS__PINOUT_STATE__OUTPUTS__LENGTH];
  uint32_t output_periods[PROTON_SIGNALS__PINOUT_STATE__OUTPUT_PERIODS__LENGTH];
} PROTON_BUNDLE__pinout_state_t;

typedef struct PROTON_BUNDLE__alerts {
  char alert_string[PROTON_SIGNALS__ALERTS__ALERT_STRING__CAPACITY];
} PROTON_BUNDLE__alerts_t;

typedef struct PROTON_BUNDLE__cmd_fans {
  uint8_t fan_speeds[PROTON_SIGNALS__CMD_FANS__FAN_SPEEDS__CAPACITY];
} PROTON_BUNDLE__cmd_fans_t;

typedef struct PROTON_BUNDLE__display_status {
  char string_1[PROTON_SIGNALS__DISPLAY_STATUS__STRING_1__CAPACITY];
  char string_2[PROTON_SIGNALS__DISPLAY_STATUS__STRING_2__CAPACITY];
} PROTON_BUNDLE__display_status_t;

typedef struct PROTON_BUNDLE__cmd_lights {
  uint8_t front_left_light[PROTON_SIGNALS__CMD_LIGHTS__FRONT_LEFT_LIGHT__CAPACITY];
  uint8_t front_right_light[PROTON_SIGNALS__CMD_LIGHTS__FRONT_RIGHT_LIGHT__CAPACITY];
  uint8_t rear_left_light[PROTON_SIGNALS__CMD_LIGHTS__REAR_LEFT_LIGHT__CAPACITY];
  uint8_t rear_right_light[PROTON_SIGNALS__CMD_LIGHTS__REAR_RIGHT_LIGHT__CAPACITY];
} PROTON_BUNDLE__cmd_lights_t;

typedef struct PROTON_BUNDLE__battery {
  float percentage;
} PROTON_BUNDLE__battery_t;

typedef struct PROTON_BUNDLE__pinout_command {
  bool rails[PROTON_SIGNALS__PINOUT_COMMAND__RAILS__LENGTH];
  uint32_t outputs[PROTON_SIGNALS__PINOUT_COMMAND__OUTPUTS__LENGTH];
} PROTON_BUNDLE__pinout_command_t;

typedef struct PROTON_BUNDLE__cmd_shutdown {
} PROTON_BUNDLE__cmd_shutdown_t;

typedef struct PROTON_BUNDLE__clear_needs_reset {
} PROTON_BUNDLE__clear_needs_reset_t;

// External Node

extern proton_node_t mcu_node;

// External Bundle Structures

extern PROTON_BUNDLE__logger_t logger_bundle;
extern PROTON_BUNDLE__status_t status_bundle;
extern PROTON_BUNDLE__power_t power_bundle;
extern PROTON_BUNDLE__emergency_stop_t emergency_stop_bundle;
extern PROTON_BUNDLE__temperature_t temperature_bundle;
extern PROTON_BUNDLE__stop_status_t stop_status_bundle;
extern PROTON_BUNDLE__pinout_state_t pinout_state_bundle;
extern PROTON_BUNDLE__alerts_t alerts_bundle;
extern PROTON_BUNDLE__cmd_fans_t cmd_fans_bundle;
extern PROTON_BUNDLE__display_status_t display_status_bundle;
extern PROTON_BUNDLE__cmd_lights_t cmd_lights_bundle;
extern PROTON_BUNDLE__battery_t battery_bundle;
extern PROTON_BUNDLE__pinout_command_t pinout_command_bundle;
extern PROTON_BUNDLE__cmd_shutdown_t cmd_shutdown_bundle;
extern PROTON_BUNDLE__clear_needs_reset_t clear_needs_reset_bundle;

// Bundle Init Prototype

void PROTON_BUNDLE_Init();

// Bundle Receive Prototype

bool PROTON_BUNDLE_Receive(const uint8_t* buffer, size_t length);

// Bundle Send Prototype

bool PROTON_BUNDLE_Send(PROTON_BUNDLE_e bundle);

// Consumer callbacks

void PROTON_BUNDLE_CmdFansCallback();
void PROTON_BUNDLE_DisplayStatusCallback();
void PROTON_BUNDLE_CmdLightsCallback();
void PROTON_BUNDLE_BatteryCallback();
void PROTON_BUNDLE_PinoutCommandCallback();
void PROTON_BUNDLE_CmdShutdownCallback();
void PROTON_BUNDLE_ClearNeedsResetCallback();

// Transport Buffers

extern proton_buffer_t proton_mcu_read_buffer;
extern proton_buffer_t proton_mcu_write_buffer;

// Transport Prototypes

bool PROTON_TRANSPORT__McuConnect();
bool PROTON_TRANSPORT__McuDisconnect();
size_t PROTON_TRANSPORT__McuRead(uint8_t * buf, size_t len);
size_t PROTON_TRANSPORT__McuWrite(const uint8_t * buf, size_t len);

// Mutex prototypes

bool PROTON_MUTEX__McuLock();
bool PROTON_MUTEX__McuUnlock();

// Proton Init Prototype

void PROTON_Init();

#endif  // PROTONC__PROTON__A300_MCU_H
