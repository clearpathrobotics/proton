
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

#include "proton__a300_mcu.h"

// Bundle Structures

PROTON_BUNDLE__logger_t logger_struct;
PROTON_BUNDLE__status_t status_struct;
PROTON_BUNDLE__power_t power_struct;
PROTON_BUNDLE__emergency_stop_t emergency_stop_struct;
PROTON_BUNDLE__temperature_t temperature_struct;
PROTON_BUNDLE__stop_status_t stop_status_struct;
PROTON_BUNDLE__pinout_state_t pinout_state_struct;
PROTON_BUNDLE__alerts_t alerts_struct;
PROTON_BUNDLE__cmd_fans_t cmd_fans_struct;
PROTON_BUNDLE__display_status_t display_status_struct;
PROTON_BUNDLE__cmd_lights_t cmd_lights_struct;
PROTON_BUNDLE__battery_t battery_struct;
PROTON_BUNDLE__pinout_command_t pinout_command_struct;
PROTON_BUNDLE__cmd_shutdown_trigger_t cmd_shutdown_trigger_struct;
PROTON_BUNDLE__cmd_shutdown_response_t cmd_shutdown_response_struct;
PROTON_BUNDLE__clear_needs_reset_trigger_t clear_needs_reset_trigger_struct;

// Signals

proton_signal_t logger_signals[PROTON_SIGNALS__LOGGER_COUNT];
proton_signal_t status_signals[PROTON_SIGNALS__STATUS_COUNT];
proton_signal_t power_signals[PROTON_SIGNALS__POWER_COUNT];
proton_signal_t emergency_stop_signals[PROTON_SIGNALS__EMERGENCY_STOP_COUNT];
proton_signal_t temperature_signals[PROTON_SIGNALS__TEMPERATURE_COUNT];
proton_signal_t stop_status_signals[PROTON_SIGNALS__STOP_STATUS_COUNT];
proton_signal_t pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE_COUNT];
proton_signal_t alerts_signals[PROTON_SIGNALS__ALERTS_COUNT];
proton_signal_t cmd_fans_signals[PROTON_SIGNALS__CMD_FANS_COUNT];
proton_signal_t display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS_COUNT];
proton_signal_t cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS_COUNT];
proton_signal_t battery_signals[PROTON_SIGNALS__BATTERY_COUNT];
proton_signal_t pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND_COUNT];
proton_signal_t cmd_shutdown_trigger_signals[PROTON_SIGNALS__CMD_SHUTDOWN_TRIGGER_COUNT];
proton_signal_t cmd_shutdown_response_signals[PROTON_SIGNALS__CMD_SHUTDOWN_RESPONSE_COUNT];
proton_signal_t clear_needs_reset_trigger_signals[PROTON_SIGNALS__CLEAR_NEEDS_RESET_TRIGGER_COUNT];

// Bundles

proton_bundle_t logger_bundle;
proton_bundle_t status_bundle;
proton_bundle_t power_bundle;
proton_bundle_t emergency_stop_bundle;
proton_bundle_t temperature_bundle;
proton_bundle_t stop_status_bundle;
proton_bundle_t pinout_state_bundle;
proton_bundle_t alerts_bundle;
proton_bundle_t cmd_fans_bundle;
proton_bundle_t display_status_bundle;
proton_bundle_t cmd_lights_bundle;
proton_bundle_t battery_bundle;
proton_bundle_t pinout_command_bundle;
proton_bundle_t cmd_shutdown_trigger_bundle;
proton_bundle_t cmd_shutdown_response_bundle;
proton_bundle_t clear_needs_reset_trigger_bundle;

// Bundle Init Prototype

void PROTON_BUNDLE_InitLogger();
void PROTON_BUNDLE_InitStatus();
void PROTON_BUNDLE_InitPower();
void PROTON_BUNDLE_InitEmergencyStop();
void PROTON_BUNDLE_InitTemperature();
void PROTON_BUNDLE_InitStopStatus();
void PROTON_BUNDLE_InitPinoutState();
void PROTON_BUNDLE_InitAlerts();
void PROTON_BUNDLE_InitCmdFans();
void PROTON_BUNDLE_InitDisplayStatus();
void PROTON_BUNDLE_InitCmdLights();
void PROTON_BUNDLE_InitBattery();
void PROTON_BUNDLE_InitPinoutCommand();
void PROTON_BUNDLE_InitCmdShutdownTrigger();
void PROTON_BUNDLE_InitCmdShutdownResponse();
void PROTON_BUNDLE_InitClearNeedsResetTrigger();

// Bundle Init Functions

void PROTON_BUNDLE_InitLogger()
{
  logger_signals[PROTON_SIGNALS__LOGGER__LEVEL].signal.which_signal = proton_Signal_uint32_value_tag;
  logger_signals[PROTON_SIGNALS__LOGGER__LEVEL].arg.data = &logger_struct.level;

  logger_signals[PROTON_SIGNALS__LOGGER__NAME].signal.which_signal = proton_Signal_string_value_tag;
  logger_signals[PROTON_SIGNALS__LOGGER__NAME].signal.signal.string_value = &logger_signals[PROTON_SIGNALS__LOGGER__NAME].arg;
  logger_signals[PROTON_SIGNALS__LOGGER__NAME].arg.data = logger_struct.name;
  logger_signals[PROTON_SIGNALS__LOGGER__NAME].arg.capacity = 64;
  logger_signals[PROTON_SIGNALS__LOGGER__NAME].arg.size = 0;

  logger_signals[PROTON_SIGNALS__LOGGER__MSG].signal.which_signal = proton_Signal_string_value_tag;
  logger_signals[PROTON_SIGNALS__LOGGER__MSG].signal.signal.string_value = &logger_signals[PROTON_SIGNALS__LOGGER__MSG].arg;
  logger_signals[PROTON_SIGNALS__LOGGER__MSG].arg.data = logger_struct.msg;
  logger_signals[PROTON_SIGNALS__LOGGER__MSG].arg.capacity = 64;
  logger_signals[PROTON_SIGNALS__LOGGER__MSG].arg.size = 0;

  logger_signals[PROTON_SIGNALS__LOGGER__FILE].signal.which_signal = proton_Signal_string_value_tag;
  logger_signals[PROTON_SIGNALS__LOGGER__FILE].signal.signal.string_value = &logger_signals[PROTON_SIGNALS__LOGGER__FILE].arg;
  logger_signals[PROTON_SIGNALS__LOGGER__FILE].arg.data = logger_struct.file;
  logger_signals[PROTON_SIGNALS__LOGGER__FILE].arg.capacity = 64;
  logger_signals[PROTON_SIGNALS__LOGGER__FILE].arg.size = 0;

  logger_signals[PROTON_SIGNALS__LOGGER__FUNCTION].signal.which_signal = proton_Signal_string_value_tag;
  logger_signals[PROTON_SIGNALS__LOGGER__FUNCTION].signal.signal.string_value = &logger_signals[PROTON_SIGNALS__LOGGER__FUNCTION].arg;
  logger_signals[PROTON_SIGNALS__LOGGER__FUNCTION].arg.data = logger_struct.function;
  logger_signals[PROTON_SIGNALS__LOGGER__FUNCTION].arg.capacity = 64;
  logger_signals[PROTON_SIGNALS__LOGGER__FUNCTION].arg.size = 0;

  logger_signals[PROTON_SIGNALS__LOGGER__LINE].signal.which_signal = proton_Signal_uint32_value_tag;
  logger_signals[PROTON_SIGNALS__LOGGER__LINE].arg.data = &logger_struct.line;

  PROTON_InitBundle(&logger_bundle, PROTON_BUNDLE_ID__LOGGER, logger_signals, PROTON_SIGNALS__LOGGER_COUNT);
}

void PROTON_BUNDLE_InitStatus()
{
  status_signals[PROTON_SIGNALS__STATUS__HARDWARE_ID].signal.which_signal = proton_Signal_string_value_tag;
  status_signals[PROTON_SIGNALS__STATUS__HARDWARE_ID].signal.signal.string_value = &status_signals[PROTON_SIGNALS__STATUS__HARDWARE_ID].arg;
  status_signals[PROTON_SIGNALS__STATUS__HARDWARE_ID].arg.data = status_struct.hardware_id;
  status_signals[PROTON_SIGNALS__STATUS__HARDWARE_ID].arg.capacity = 10;
  status_signals[PROTON_SIGNALS__STATUS__HARDWARE_ID].arg.size = 0;

  status_signals[PROTON_SIGNALS__STATUS__FIRMWARE_VERSION].signal.which_signal = proton_Signal_string_value_tag;
  status_signals[PROTON_SIGNALS__STATUS__FIRMWARE_VERSION].signal.signal.string_value = &status_signals[PROTON_SIGNALS__STATUS__FIRMWARE_VERSION].arg;
  status_signals[PROTON_SIGNALS__STATUS__FIRMWARE_VERSION].arg.data = status_struct.firmware_version;
  status_signals[PROTON_SIGNALS__STATUS__FIRMWARE_VERSION].arg.capacity = 10;
  status_signals[PROTON_SIGNALS__STATUS__FIRMWARE_VERSION].arg.size = 0;

  status_signals[PROTON_SIGNALS__STATUS__MCU_UPTIME_S].signal.which_signal = proton_Signal_uint32_value_tag;
  status_signals[PROTON_SIGNALS__STATUS__MCU_UPTIME_S].arg.data = &status_struct.mcu_uptime_s;

  status_signals[PROTON_SIGNALS__STATUS__MCU_UPTIME_NS].signal.which_signal = proton_Signal_uint32_value_tag;
  status_signals[PROTON_SIGNALS__STATUS__MCU_UPTIME_NS].arg.data = &status_struct.mcu_uptime_ns;

  status_signals[PROTON_SIGNALS__STATUS__CONNECTION_UPTIME_S].signal.which_signal = proton_Signal_uint32_value_tag;
  status_signals[PROTON_SIGNALS__STATUS__CONNECTION_UPTIME_S].arg.data = &status_struct.connection_uptime_s;

  status_signals[PROTON_SIGNALS__STATUS__CONNECTION_UPTIME_NS].signal.which_signal = proton_Signal_uint32_value_tag;
  status_signals[PROTON_SIGNALS__STATUS__CONNECTION_UPTIME_NS].arg.data = &status_struct.connection_uptime_ns;

  PROTON_InitBundle(&status_bundle, PROTON_BUNDLE_ID__STATUS, status_signals, PROTON_SIGNALS__STATUS_COUNT);
}

void PROTON_BUNDLE_InitPower()
{
  power_signals[PROTON_SIGNALS__POWER__MEASURED_VOLTAGES].signal.which_signal = proton_Signal_list_float_value_tag;
  power_signals[PROTON_SIGNALS__POWER__MEASURED_VOLTAGES].signal.signal.list_float_value.floats = &power_signals[PROTON_SIGNALS__POWER__MEASURED_VOLTAGES].arg;
  power_signals[PROTON_SIGNALS__POWER__MEASURED_VOLTAGES].arg.data = power_struct.measured_voltages;
  power_signals[PROTON_SIGNALS__POWER__MEASURED_VOLTAGES].arg.capacity = 13;
  power_signals[PROTON_SIGNALS__POWER__MEASURED_VOLTAGES].arg.size = 0;

  power_signals[PROTON_SIGNALS__POWER__MEASURED_CURRENTS].signal.which_signal = proton_Signal_list_float_value_tag;
  power_signals[PROTON_SIGNALS__POWER__MEASURED_CURRENTS].signal.signal.list_float_value.floats = &power_signals[PROTON_SIGNALS__POWER__MEASURED_CURRENTS].arg;
  power_signals[PROTON_SIGNALS__POWER__MEASURED_CURRENTS].arg.data = power_struct.measured_currents;
  power_signals[PROTON_SIGNALS__POWER__MEASURED_CURRENTS].arg.capacity = 13;
  power_signals[PROTON_SIGNALS__POWER__MEASURED_CURRENTS].arg.size = 0;

  PROTON_InitBundle(&power_bundle, PROTON_BUNDLE_ID__POWER, power_signals, PROTON_SIGNALS__POWER_COUNT);
}

void PROTON_BUNDLE_InitEmergencyStop()
{
  emergency_stop_signals[PROTON_SIGNALS__EMERGENCY_STOP__STOPPED].signal.which_signal = proton_Signal_bool_value_tag;
  emergency_stop_signals[PROTON_SIGNALS__EMERGENCY_STOP__STOPPED].arg.data = &emergency_stop_struct.stopped;

  PROTON_InitBundle(&emergency_stop_bundle, PROTON_BUNDLE_ID__EMERGENCY_STOP, emergency_stop_signals, PROTON_SIGNALS__EMERGENCY_STOP_COUNT);
}

void PROTON_BUNDLE_InitTemperature()
{
  temperature_signals[PROTON_SIGNALS__TEMPERATURE__TEMPERATURES].signal.which_signal = proton_Signal_list_float_value_tag;
  temperature_signals[PROTON_SIGNALS__TEMPERATURE__TEMPERATURES].signal.signal.list_float_value.floats = &temperature_signals[PROTON_SIGNALS__TEMPERATURE__TEMPERATURES].arg;
  temperature_signals[PROTON_SIGNALS__TEMPERATURE__TEMPERATURES].arg.data = temperature_struct.temperatures;
  temperature_signals[PROTON_SIGNALS__TEMPERATURE__TEMPERATURES].arg.capacity = 18;
  temperature_signals[PROTON_SIGNALS__TEMPERATURE__TEMPERATURES].arg.size = 0;

  PROTON_InitBundle(&temperature_bundle, PROTON_BUNDLE_ID__TEMPERATURE, temperature_signals, PROTON_SIGNALS__TEMPERATURE_COUNT);
}

void PROTON_BUNDLE_InitStopStatus()
{
  stop_status_signals[PROTON_SIGNALS__STOP_STATUS__NEEDS_RESET].signal.which_signal = proton_Signal_bool_value_tag;
  stop_status_signals[PROTON_SIGNALS__STOP_STATUS__NEEDS_RESET].arg.data = &stop_status_struct.needs_reset;

  PROTON_InitBundle(&stop_status_bundle, PROTON_BUNDLE_ID__STOP_STATUS, stop_status_signals, PROTON_SIGNALS__STOP_STATUS_COUNT);
}

void PROTON_BUNDLE_InitPinoutState()
{
  pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__RAILS].signal.which_signal = proton_Signal_list_bool_value_tag;
  pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__RAILS].signal.signal.list_bool_value.bools = &pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__RAILS].arg;
  pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__RAILS].arg.data = pinout_state_struct.rails;
  pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__RAILS].arg.capacity = 1;
  pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__RAILS].arg.size = 0;

  pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__INPUTS].signal.which_signal = proton_Signal_list_bool_value_tag;
  pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__INPUTS].signal.signal.list_bool_value.bools = &pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__INPUTS].arg;
  pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__INPUTS].arg.data = pinout_state_struct.inputs;
  pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__INPUTS].arg.capacity = 7;
  pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__INPUTS].arg.size = 0;

  pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUTS].signal.which_signal = proton_Signal_list_bool_value_tag;
  pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUTS].signal.signal.list_bool_value.bools = &pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUTS].arg;
  pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUTS].arg.data = pinout_state_struct.outputs;
  pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUTS].arg.capacity = 7;
  pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUTS].arg.size = 0;

  pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUT_PERIODS].signal.which_signal = proton_Signal_list_uint32_value_tag;
  pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUT_PERIODS].signal.signal.list_uint32_value.uint32s = &pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUT_PERIODS].arg;
  pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUT_PERIODS].arg.data = pinout_state_struct.output_periods;
  pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUT_PERIODS].arg.capacity = 7;
  pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUT_PERIODS].arg.size = 0;

  PROTON_InitBundle(&pinout_state_bundle, PROTON_BUNDLE_ID__PINOUT_STATE, pinout_state_signals, PROTON_SIGNALS__PINOUT_STATE_COUNT);
}

void PROTON_BUNDLE_InitAlerts()
{
  alerts_signals[PROTON_SIGNALS__ALERTS__ALERT_STRING].signal.which_signal = proton_Signal_string_value_tag;
  alerts_signals[PROTON_SIGNALS__ALERTS__ALERT_STRING].signal.signal.string_value = &alerts_signals[PROTON_SIGNALS__ALERTS__ALERT_STRING].arg;
  alerts_signals[PROTON_SIGNALS__ALERTS__ALERT_STRING].arg.data = alerts_struct.alert_string;
  alerts_signals[PROTON_SIGNALS__ALERTS__ALERT_STRING].arg.capacity = 86;
  alerts_signals[PROTON_SIGNALS__ALERTS__ALERT_STRING].arg.size = 0;

  PROTON_InitBundle(&alerts_bundle, PROTON_BUNDLE_ID__ALERTS, alerts_signals, PROTON_SIGNALS__ALERTS_COUNT);
}

void PROTON_BUNDLE_InitCmdFans()
{
  cmd_fans_signals[PROTON_SIGNALS__CMD_FANS__FAN_SPEEDS].signal.which_signal = proton_Signal_bytes_value_tag;
  cmd_fans_signals[PROTON_SIGNALS__CMD_FANS__FAN_SPEEDS].signal.signal.bytes_value = &cmd_fans_signals[PROTON_SIGNALS__CMD_FANS__FAN_SPEEDS].arg;
  cmd_fans_signals[PROTON_SIGNALS__CMD_FANS__FAN_SPEEDS].arg.data = cmd_fans_struct.fan_speeds;
  cmd_fans_signals[PROTON_SIGNALS__CMD_FANS__FAN_SPEEDS].arg.capacity = 8;
  cmd_fans_signals[PROTON_SIGNALS__CMD_FANS__FAN_SPEEDS].arg.size = 0;

  PROTON_InitBundle(&cmd_fans_bundle, PROTON_BUNDLE_ID__CMD_FANS, cmd_fans_signals, PROTON_SIGNALS__CMD_FANS_COUNT);
}

void PROTON_BUNDLE_InitDisplayStatus()
{
  display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_1].signal.which_signal = proton_Signal_string_value_tag;
  display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_1].signal.signal.string_value = &display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_1].arg;
  display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_1].arg.data = display_status_struct.string_1;
  display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_1].arg.capacity = 50;
  display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_1].arg.size = 0;

  display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_2].signal.which_signal = proton_Signal_string_value_tag;
  display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_2].signal.signal.string_value = &display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_2].arg;
  display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_2].arg.data = display_status_struct.string_2;
  display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_2].arg.capacity = 50;
  display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_2].arg.size = 0;

  PROTON_InitBundle(&display_status_bundle, PROTON_BUNDLE_ID__DISPLAY_STATUS, display_status_signals, PROTON_SIGNALS__DISPLAY_STATUS_COUNT);
}

void PROTON_BUNDLE_InitCmdLights()
{
  cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_LEFT_LIGHT].signal.which_signal = proton_Signal_bytes_value_tag;
  cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_LEFT_LIGHT].signal.signal.bytes_value = &cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_LEFT_LIGHT].arg;
  cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_LEFT_LIGHT].arg.data = cmd_lights_struct.front_left_light;
  cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_LEFT_LIGHT].arg.capacity = 3;
  cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_LEFT_LIGHT].arg.size = 0;

  cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_RIGHT_LIGHT].signal.which_signal = proton_Signal_bytes_value_tag;
  cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_RIGHT_LIGHT].signal.signal.bytes_value = &cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_RIGHT_LIGHT].arg;
  cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_RIGHT_LIGHT].arg.data = cmd_lights_struct.front_right_light;
  cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_RIGHT_LIGHT].arg.capacity = 3;
  cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_RIGHT_LIGHT].arg.size = 0;

  cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_LEFT_LIGHT].signal.which_signal = proton_Signal_bytes_value_tag;
  cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_LEFT_LIGHT].signal.signal.bytes_value = &cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_LEFT_LIGHT].arg;
  cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_LEFT_LIGHT].arg.data = cmd_lights_struct.rear_left_light;
  cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_LEFT_LIGHT].arg.capacity = 3;
  cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_LEFT_LIGHT].arg.size = 0;

  cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_RIGHT_LIGHT].signal.which_signal = proton_Signal_bytes_value_tag;
  cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_RIGHT_LIGHT].signal.signal.bytes_value = &cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_RIGHT_LIGHT].arg;
  cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_RIGHT_LIGHT].arg.data = cmd_lights_struct.rear_right_light;
  cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_RIGHT_LIGHT].arg.capacity = 3;
  cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_RIGHT_LIGHT].arg.size = 0;

  PROTON_InitBundle(&cmd_lights_bundle, PROTON_BUNDLE_ID__CMD_LIGHTS, cmd_lights_signals, PROTON_SIGNALS__CMD_LIGHTS_COUNT);
}

void PROTON_BUNDLE_InitBattery()
{
  battery_signals[PROTON_SIGNALS__BATTERY__PERCENTAGE].signal.which_signal = proton_Signal_float_value_tag;
  battery_signals[PROTON_SIGNALS__BATTERY__PERCENTAGE].arg.data = &battery_struct.percentage;

  PROTON_InitBundle(&battery_bundle, PROTON_BUNDLE_ID__BATTERY, battery_signals, PROTON_SIGNALS__BATTERY_COUNT);
}

void PROTON_BUNDLE_InitPinoutCommand()
{
  pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__RAILS].signal.which_signal = proton_Signal_list_bool_value_tag;
  pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__RAILS].signal.signal.list_bool_value.bools = &pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__RAILS].arg;
  pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__RAILS].arg.data = pinout_command_struct.rails;
  pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__RAILS].arg.capacity = 1;
  pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__RAILS].arg.size = 0;

  pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__OUTPUTS].signal.which_signal = proton_Signal_list_uint32_value_tag;
  pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__OUTPUTS].signal.signal.list_uint32_value.uint32s = &pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__OUTPUTS].arg;
  pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__OUTPUTS].arg.data = pinout_command_struct.outputs;
  pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__OUTPUTS].arg.capacity = 7;
  pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__OUTPUTS].arg.size = 0;

  PROTON_InitBundle(&pinout_command_bundle, PROTON_BUNDLE_ID__PINOUT_COMMAND, pinout_command_signals, PROTON_SIGNALS__PINOUT_COMMAND_COUNT);
}

void PROTON_BUNDLE_InitCmdShutdownTrigger()
{
  cmd_shutdown_trigger_signals[PROTON_SIGNALS__CMD_SHUTDOWN_TRIGGER__SHUTDOWN_COMMAND].signal.which_signal = proton_Signal_uint32_value_tag;
  cmd_shutdown_trigger_signals[PROTON_SIGNALS__CMD_SHUTDOWN_TRIGGER__SHUTDOWN_COMMAND].arg.data = &cmd_shutdown_trigger_struct.shutdown_command;

  PROTON_InitBundle(&cmd_shutdown_trigger_bundle, PROTON_BUNDLE_ID__CMD_SHUTDOWN_TRIGGER, cmd_shutdown_trigger_signals, PROTON_SIGNALS__CMD_SHUTDOWN_TRIGGER_COUNT);
}

void PROTON_BUNDLE_InitCmdShutdownResponse()
{
  cmd_shutdown_response_signals[PROTON_SIGNALS__CMD_SHUTDOWN_RESPONSE__SHUTDOWN_COMMAND_RESPONSE].signal.which_signal = proton_Signal_uint32_value_tag;
  cmd_shutdown_response_signals[PROTON_SIGNALS__CMD_SHUTDOWN_RESPONSE__SHUTDOWN_COMMAND_RESPONSE].arg.data = &cmd_shutdown_response_struct.shutdown_command_response;

  PROTON_InitBundle(&cmd_shutdown_response_bundle, PROTON_BUNDLE_ID__CMD_SHUTDOWN_RESPONSE, cmd_shutdown_response_signals, PROTON_SIGNALS__CMD_SHUTDOWN_RESPONSE_COUNT);
}

void PROTON_BUNDLE_InitClearNeedsResetTrigger()
{
  clear_needs_reset_trigger_signals[PROTON_SIGNALS__CLEAR_NEEDS_RESET_TRIGGER__CLEAR_NEEDS_RESET].signal.which_signal = proton_Signal_bool_value_tag;
  clear_needs_reset_trigger_signals[PROTON_SIGNALS__CLEAR_NEEDS_RESET_TRIGGER__CLEAR_NEEDS_RESET].arg.data = &clear_needs_reset_trigger_struct.clear_needs_reset;

  PROTON_InitBundle(&clear_needs_reset_trigger_bundle, PROTON_BUNDLE_ID__CLEAR_NEEDS_RESET_TRIGGER, clear_needs_reset_trigger_signals, PROTON_SIGNALS__CLEAR_NEEDS_RESET_TRIGGER_COUNT);
}

void PROTON_BUNDLE_Init()
{
  PROTON_BUNDLE_InitLogger();
  PROTON_BUNDLE_InitStatus();
  PROTON_BUNDLE_InitPower();
  PROTON_BUNDLE_InitEmergencyStop();
  PROTON_BUNDLE_InitTemperature();
  PROTON_BUNDLE_InitStopStatus();
  PROTON_BUNDLE_InitPinoutState();
  PROTON_BUNDLE_InitAlerts();
  PROTON_BUNDLE_InitCmdFans();
  PROTON_BUNDLE_InitDisplayStatus();
  PROTON_BUNDLE_InitCmdLights();
  PROTON_BUNDLE_InitBattery();
  PROTON_BUNDLE_InitPinoutCommand();
  PROTON_BUNDLE_InitCmdShutdownTrigger();
  PROTON_BUNDLE_InitCmdShutdownResponse();
  PROTON_BUNDLE_InitClearNeedsResetTrigger();
}

// Bundle Decode Function

bool PROTON_BUNDLE_Decode(const uint8_t * buffer, size_t length)
{
  proton_bundle_t * bundle;
  uint32_t id;
  proton_callback_t callback;

  // Decode bundle ID
  if (!PROTON_DecodeId(&id, buffer, length))
  {
    return false;
  }

  switch (id)
  {
    case PROTON_BUNDLE_ID__CMD_FANS:
    {
      bundle = &cmd_fans_bundle;
      callback = PROTON_BUNDLE_CmdFansCallback;
      break;
    }

    case PROTON_BUNDLE_ID__DISPLAY_STATUS:
    {
      bundle = &display_status_bundle;
      callback = PROTON_BUNDLE_DisplayStatusCallback;
      break;
    }

    case PROTON_BUNDLE_ID__CMD_LIGHTS:
    {
      bundle = &cmd_lights_bundle;
      callback = PROTON_BUNDLE_CmdLightsCallback;
      break;
    }

    case PROTON_BUNDLE_ID__BATTERY:
    {
      bundle = &battery_bundle;
      callback = PROTON_BUNDLE_BatteryCallback;
      break;
    }

    case PROTON_BUNDLE_ID__PINOUT_COMMAND:
    {
      bundle = &pinout_command_bundle;
      callback = PROTON_BUNDLE_PinoutCommandCallback;
      break;
    }

    case PROTON_BUNDLE_ID__CMD_SHUTDOWN_TRIGGER:
    {
      bundle = &cmd_shutdown_trigger_bundle;
      callback = PROTON_BUNDLE_CmdShutdownTriggerCallback;
      break;
    }

    case PROTON_BUNDLE_ID__CLEAR_NEEDS_RESET_TRIGGER:
    {
      bundle = &clear_needs_reset_trigger_bundle;
      callback = PROTON_BUNDLE_ClearNeedsResetTriggerCallback;
      break;
    }

    default:
    {
      return false;
    }
  }

  // Decode bundle
  if (PROTON_Decode(bundle, buffer, length) != 0)
  {
    return false;
  }

  // Execute callback
  if (callback)
  {
    callback();
  }

  return true;
}

