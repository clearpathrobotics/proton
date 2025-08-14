
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

#ifndef PROTONC__PROTON__A300_H
#define PROTONC__PROTON__A300_H

#include "stdint.h"
#include "stdbool.h"
#include "proton.h"

// Signal Enums

typedef enum PROTON_SIGNALS__logger {
  PROTON_SIGNALS__LOGGER__LOG,
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

typedef enum PROTON_SIGNALS__error_codes {
  PROTON_SIGNALS__ERROR_CODES__ERRORS,
  PROTON_SIGNALS__ERROR_CODES_COUNT
} PROTON_SIGNALS__error_codes_e;

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


// Message Structure Definitions

typedef struct PROTON_BUNDLE__logger {
  char log[64];
} PROTON_BUNDLE__logger_t;

typedef struct PROTON_BUNDLE__status {
  char hardware_id[64];
  char firmware_version[64];
  uint32_t mcu_uptime_s;
  uint32_t mcu_uptime_ns;
  uint32_t connection_uptime_s;
  uint32_t connection_uptime_ns;
} PROTON_BUNDLE__status_t;

typedef struct PROTON_BUNDLE__power {
  float measured_voltages[13];
  float measured_currents[13];
} PROTON_BUNDLE__power_t;

typedef struct PROTON_BUNDLE__emergency_stop {
  bool stopped;
} PROTON_BUNDLE__emergency_stop_t;

typedef struct PROTON_BUNDLE__temperature {
  float temperatures[18];
} PROTON_BUNDLE__temperature_t;

typedef struct PROTON_BUNDLE__stop_status {
  bool needs_reset;
} PROTON_BUNDLE__stop_status_t;

typedef struct PROTON_BUNDLE__pinout_state {
  bool rails[1];
  bool inputs[7];
  bool outputs[7];
  uint32_t output_periods[7];
} PROTON_BUNDLE__pinout_state_t;

typedef struct PROTON_BUNDLE__error_codes {
  char errors[86];
} PROTON_BUNDLE__error_codes_t;

typedef struct PROTON_BUNDLE__cmd_fans {
  uint8_t fan_speeds[8];
} PROTON_BUNDLE__cmd_fans_t;

typedef struct PROTON_BUNDLE__display_status {
  char string_1[50];
  char string_2[50];
} PROTON_BUNDLE__display_status_t;

typedef struct PROTON_BUNDLE__cmd_lights {
  uint8_t front_left_light[3];
  uint8_t front_right_light[3];
  uint8_t rear_left_light[3];
  uint8_t rear_right_light[3];
} PROTON_BUNDLE__cmd_lights_t;

typedef struct PROTON_BUNDLE__battery {
  float percentage;
} PROTON_BUNDLE__battery_t;

typedef struct PROTON_BUNDLE__pinout_command {
  bool rails[1];
  uint32_t outputs[7];
} PROTON_BUNDLE__pinout_command_t;

// External Message Structures

extern PROTON_BUNDLE__logger_t logger_struct;
extern PROTON_BUNDLE__status_t status_struct;
extern PROTON_BUNDLE__power_t power_struct;
extern PROTON_BUNDLE__emergency_stop_t emergency_stop_struct;
extern PROTON_BUNDLE__temperature_t temperature_struct;
extern PROTON_BUNDLE__stop_status_t stop_status_struct;
extern PROTON_BUNDLE__pinout_state_t pinout_state_struct;
extern PROTON_BUNDLE__error_codes_t error_codes_struct;
extern PROTON_BUNDLE__cmd_fans_t cmd_fans_struct;
extern PROTON_BUNDLE__display_status_t display_status_struct;
extern PROTON_BUNDLE__cmd_lights_t cmd_lights_struct;
extern PROTON_BUNDLE__battery_t battery_struct;
extern PROTON_BUNDLE__pinout_command_t pinout_command_struct;

// External Bundles

extern proton_bundle_t logger_bundle;
extern proton_bundle_t status_bundle;
extern proton_bundle_t power_bundle;
extern proton_bundle_t emergency_stop_bundle;
extern proton_bundle_t temperature_bundle;
extern proton_bundle_t stop_status_bundle;
extern proton_bundle_t pinout_state_bundle;
extern proton_bundle_t error_codes_bundle;
extern proton_bundle_t cmd_fans_bundle;
extern proton_bundle_t display_status_bundle;
extern proton_bundle_t cmd_lights_bundle;
extern proton_bundle_t battery_bundle;
extern proton_bundle_t pinout_command_bundle;

// Message Init Prototypes

void PROTON_MESSAGE_init();

#endif  // PROTONC__PROTON__A300_H
