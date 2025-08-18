
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

// Bundle IDs

#define PROTON_BUNDLE_ID__LOGGER 0x100
#define PROTON_BUNDLE_ID__STATUS 0x101
#define PROTON_BUNDLE_ID__POWER 0x102
#define PROTON_BUNDLE_ID__EMERGENCY_STOP 0x103
#define PROTON_BUNDLE_ID__TEMPERATURE 0x104
#define PROTON_BUNDLE_ID__STOP_STATUS 0x105
#define PROTON_BUNDLE_ID__PINOUT_STATE 0x106
#define PROTON_BUNDLE_ID__ALERTS 0x107
#define PROTON_BUNDLE_ID__CMD_FANS 0x200
#define PROTON_BUNDLE_ID__DISPLAY_STATUS 0x201
#define PROTON_BUNDLE_ID__CMD_LIGHTS 0x202
#define PROTON_BUNDLE_ID__BATTERY 0x203
#define PROTON_BUNDLE_ID__PINOUT_COMMAND 0x204
#define PROTON_BUNDLE_ID__CMD_SHUTDOWN_TRIGGER 0x300
#define PROTON_BUNDLE_ID__CMD_SHUTDOWN_RESPONSE 0x300
#define PROTON_BUNDLE_ID__CLEAR_NEEDS_RESET_TRIGGER 0x301

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

typedef enum PROTON_SIGNALS__cmd_shutdown_trigger {
  PROTON_SIGNALS__CMD_SHUTDOWN_TRIGGER__SHUTDOWN_COMMAND,
  PROTON_SIGNALS__CMD_SHUTDOWN_TRIGGER_COUNT
} PROTON_SIGNALS__cmd_shutdown_trigger_e;

typedef enum PROTON_SIGNALS__cmd_shutdown_response {
  PROTON_SIGNALS__CMD_SHUTDOWN_RESPONSE__SHUTDOWN_COMMAND_RESPONSE,
  PROTON_SIGNALS__CMD_SHUTDOWN_RESPONSE_COUNT
} PROTON_SIGNALS__cmd_shutdown_response_e;

typedef enum PROTON_SIGNALS__clear_needs_reset_trigger {
  PROTON_SIGNALS__CLEAR_NEEDS_RESET_TRIGGER__CLEAR_NEEDS_RESET,
  PROTON_SIGNALS__CLEAR_NEEDS_RESET_TRIGGER_COUNT
} PROTON_SIGNALS__clear_needs_reset_trigger_e;


// Constant definitions

#define LOGGER__NAME__CAPACITY 64
#define LOGGER__MSG__CAPACITY 64
#define LOGGER__FILE__CAPACITY 64
#define LOGGER__FUNCTION__CAPACITY 64

#define STATUS__HARDWARE_ID__CAPACITY 10
#define STATUS__FIRMWARE_VERSION__CAPACITY 10

#define POWER__MEASURED_VOLTAGES__LENGTH 13
#define POWER__MEASURED_CURRENTS__LENGTH 13

#define TEMPERATURE__TEMPERATURES__LENGTH 18

#define PINOUT_STATE__RAILS__LENGTH 1
#define PINOUT_STATE__INPUTS__LENGTH 7
#define PINOUT_STATE__OUTPUTS__LENGTH 7
#define PINOUT_STATE__OUTPUT_PERIODS__LENGTH 7

#define ALERTS__ALERT_STRING__CAPACITY 86

#define CMD_FANS__FAN_SPEEDS__CAPACITY 8

#define DISPLAY_STATUS__STRING_1__CAPACITY 50
#define DISPLAY_STATUS__STRING_2__CAPACITY 50

#define CMD_LIGHTS__FRONT_LEFT_LIGHT__CAPACITY 3
#define CMD_LIGHTS__FRONT_RIGHT_LIGHT__CAPACITY 3
#define CMD_LIGHTS__REAR_LEFT_LIGHT__CAPACITY 3
#define CMD_LIGHTS__REAR_RIGHT_LIGHT__CAPACITY 3

#define PINOUT_COMMAND__RAILS__LENGTH 1
#define PINOUT_COMMAND__OUTPUTS__LENGTH 7

// Bundle Structure Definitions

typedef struct PROTON_BUNDLE__logger {
  uint32_t level;
  char name[LOGGER__NAME__CAPACITY];
  char msg[LOGGER__MSG__CAPACITY];
  char file[LOGGER__FILE__CAPACITY];
  char function[LOGGER__FUNCTION__CAPACITY];
  uint32_t line;
} PROTON_BUNDLE__logger_t;

typedef struct PROTON_BUNDLE__status {
  char hardware_id[STATUS__HARDWARE_ID__CAPACITY];
  char firmware_version[STATUS__FIRMWARE_VERSION__CAPACITY];
  uint32_t mcu_uptime_s;
  uint32_t mcu_uptime_ns;
  uint32_t connection_uptime_s;
  uint32_t connection_uptime_ns;
} PROTON_BUNDLE__status_t;

typedef struct PROTON_BUNDLE__power {
  float measured_voltages[POWER__MEASURED_VOLTAGES__LENGTH];
  float measured_currents[POWER__MEASURED_CURRENTS__LENGTH];
} PROTON_BUNDLE__power_t;

typedef struct PROTON_BUNDLE__emergency_stop {
  bool stopped;
} PROTON_BUNDLE__emergency_stop_t;

typedef struct PROTON_BUNDLE__temperature {
  float temperatures[TEMPERATURE__TEMPERATURES__LENGTH];
} PROTON_BUNDLE__temperature_t;

typedef struct PROTON_BUNDLE__stop_status {
  bool needs_reset;
} PROTON_BUNDLE__stop_status_t;

typedef struct PROTON_BUNDLE__pinout_state {
  bool rails[PINOUT_STATE__RAILS__LENGTH];
  bool inputs[PINOUT_STATE__INPUTS__LENGTH];
  bool outputs[PINOUT_STATE__OUTPUTS__LENGTH];
  uint32_t output_periods[PINOUT_STATE__OUTPUT_PERIODS__LENGTH];
} PROTON_BUNDLE__pinout_state_t;

typedef struct PROTON_BUNDLE__alerts {
  char alert_string[ALERTS__ALERT_STRING__CAPACITY];
} PROTON_BUNDLE__alerts_t;

typedef struct PROTON_BUNDLE__cmd_fans {
  uint8_t fan_speeds[CMD_FANS__FAN_SPEEDS__CAPACITY];
} PROTON_BUNDLE__cmd_fans_t;

typedef struct PROTON_BUNDLE__display_status {
  char string_1[DISPLAY_STATUS__STRING_1__CAPACITY];
  char string_2[DISPLAY_STATUS__STRING_2__CAPACITY];
} PROTON_BUNDLE__display_status_t;

typedef struct PROTON_BUNDLE__cmd_lights {
  uint8_t front_left_light[CMD_LIGHTS__FRONT_LEFT_LIGHT__CAPACITY];
  uint8_t front_right_light[CMD_LIGHTS__FRONT_RIGHT_LIGHT__CAPACITY];
  uint8_t rear_left_light[CMD_LIGHTS__REAR_LEFT_LIGHT__CAPACITY];
  uint8_t rear_right_light[CMD_LIGHTS__REAR_RIGHT_LIGHT__CAPACITY];
} PROTON_BUNDLE__cmd_lights_t;

typedef struct PROTON_BUNDLE__battery {
  float percentage;
} PROTON_BUNDLE__battery_t;

typedef struct PROTON_BUNDLE__pinout_command {
  bool rails[PINOUT_COMMAND__RAILS__LENGTH];
  uint32_t outputs[PINOUT_COMMAND__OUTPUTS__LENGTH];
} PROTON_BUNDLE__pinout_command_t;

typedef struct PROTON_BUNDLE__cmd_shutdown_trigger {
  uint32_t shutdown_command;
} PROTON_BUNDLE__cmd_shutdown_trigger_t;

typedef struct PROTON_BUNDLE__cmd_shutdown_response {
  uint32_t shutdown_command_response;
} PROTON_BUNDLE__cmd_shutdown_response_t;

typedef struct PROTON_BUNDLE__clear_needs_reset_trigger {
  bool clear_needs_reset;
} PROTON_BUNDLE__clear_needs_reset_trigger_t;

// External Bundle Structures

extern PROTON_BUNDLE__logger_t logger_struct;
extern PROTON_BUNDLE__status_t status_struct;
extern PROTON_BUNDLE__power_t power_struct;
extern PROTON_BUNDLE__emergency_stop_t emergency_stop_struct;
extern PROTON_BUNDLE__temperature_t temperature_struct;
extern PROTON_BUNDLE__stop_status_t stop_status_struct;
extern PROTON_BUNDLE__pinout_state_t pinout_state_struct;
extern PROTON_BUNDLE__alerts_t alerts_struct;
extern PROTON_BUNDLE__cmd_fans_t cmd_fans_struct;
extern PROTON_BUNDLE__display_status_t display_status_struct;
extern PROTON_BUNDLE__cmd_lights_t cmd_lights_struct;
extern PROTON_BUNDLE__battery_t battery_struct;
extern PROTON_BUNDLE__pinout_command_t pinout_command_struct;
extern PROTON_BUNDLE__cmd_shutdown_trigger_t cmd_shutdown_trigger_struct;
extern PROTON_BUNDLE__cmd_shutdown_response_t cmd_shutdown_response_struct;
extern PROTON_BUNDLE__clear_needs_reset_trigger_t clear_needs_reset_trigger_struct;

// External Bundles

extern proton_bundle_t logger_bundle;
extern proton_bundle_t status_bundle;
extern proton_bundle_t power_bundle;
extern proton_bundle_t emergency_stop_bundle;
extern proton_bundle_t temperature_bundle;
extern proton_bundle_t stop_status_bundle;
extern proton_bundle_t pinout_state_bundle;
extern proton_bundle_t alerts_bundle;
extern proton_bundle_t cmd_fans_bundle;
extern proton_bundle_t display_status_bundle;
extern proton_bundle_t cmd_lights_bundle;
extern proton_bundle_t battery_bundle;
extern proton_bundle_t pinout_command_bundle;
extern proton_bundle_t cmd_shutdown_trigger_bundle;
extern proton_bundle_t cmd_shutdown_response_bundle;
extern proton_bundle_t clear_needs_reset_trigger_bundle;

// Bundle Init Prototype

void PROTON_BUNDLE_Init();

// Bundle Decode Prototype

bool PROTON_BUNDLE_Decode(const uint8_t * buffer, size_t length);

// Consumer callbacks

void PROTON_BUNDLE_CmdFansCallback();
void PROTON_BUNDLE_DisplayStatusCallback();
void PROTON_BUNDLE_CmdLightsCallback();
void PROTON_BUNDLE_BatteryCallback();
void PROTON_BUNDLE_PinoutCommandCallback();
void PROTON_BUNDLE_CmdShutdownTriggerCallback();
void PROTON_BUNDLE_ClearNeedsResetTriggerCallback();

#endif  // PROTONC__PROTON__A300_MCU_H
