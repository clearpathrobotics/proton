
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

// Node

proton_node_t mcu_node;

// External Bundle Structures

PROTON_BUNDLE__logger_t logger_bundle;
PROTON_BUNDLE__status_t status_bundle;
PROTON_BUNDLE__power_t power_bundle;
PROTON_BUNDLE__emergency_stop_t emergency_stop_bundle;
PROTON_BUNDLE__temperature_t temperature_bundle;
PROTON_BUNDLE__stop_status_t stop_status_bundle;
PROTON_BUNDLE__pinout_state_t pinout_state_bundle;
PROTON_BUNDLE__alerts_t alerts_bundle;
PROTON_BUNDLE__cmd_fans_t cmd_fans_bundle;
PROTON_BUNDLE__display_status_t display_status_bundle;
PROTON_BUNDLE__cmd_lights_t cmd_lights_bundle;
PROTON_BUNDLE__battery_t battery_bundle;
PROTON_BUNDLE__pinout_command_t pinout_command_bundle;
PROTON_BUNDLE__cmd_shutdown_t cmd_shutdown_bundle;
PROTON_BUNDLE__clear_needs_reset_t clear_needs_reset_bundle;

// Internal Signals

static proton_signal_t _logger_signals[PROTON_SIGNALS__LOGGER_COUNT];
static proton_signal_t _status_signals[PROTON_SIGNALS__STATUS_COUNT];
static proton_signal_t _power_signals[PROTON_SIGNALS__POWER_COUNT];
static proton_signal_t _emergency_stop_signals[PROTON_SIGNALS__EMERGENCY_STOP_COUNT];
static proton_signal_t _temperature_signals[PROTON_SIGNALS__TEMPERATURE_COUNT];
static proton_signal_t _stop_status_signals[PROTON_SIGNALS__STOP_STATUS_COUNT];
static proton_signal_t _pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE_COUNT];
static proton_signal_t _alerts_signals[PROTON_SIGNALS__ALERTS_COUNT];
static proton_signal_t _cmd_fans_signals[PROTON_SIGNALS__CMD_FANS_COUNT];
static proton_signal_t _display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS_COUNT];
static proton_signal_t _cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS_COUNT];
static proton_signal_t _battery_signals[PROTON_SIGNALS__BATTERY_COUNT];
static proton_signal_t _pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND_COUNT];
static proton_signal_t _cmd_shutdown_signals[PROTON_SIGNALS__CMD_SHUTDOWN_COUNT];
static proton_signal_t _clear_needs_reset_signals[PROTON_SIGNALS__CLEAR_NEEDS_RESET_COUNT];

// Internal Bundles

static proton_bundle_t _logger_bundle;
static proton_bundle_t _status_bundle;
static proton_bundle_t _power_bundle;
static proton_bundle_t _emergency_stop_bundle;
static proton_bundle_t _temperature_bundle;
static proton_bundle_t _stop_status_bundle;
static proton_bundle_t _pinout_state_bundle;
static proton_bundle_t _alerts_bundle;
static proton_bundle_t _cmd_fans_bundle;
static proton_bundle_t _display_status_bundle;
static proton_bundle_t _cmd_lights_bundle;
static proton_bundle_t _battery_bundle;
static proton_bundle_t _pinout_command_bundle;
static proton_bundle_t _cmd_shutdown_bundle;
static proton_bundle_t _clear_needs_reset_bundle;

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
void PROTON_BUNDLE_InitCmdShutdown();
void PROTON_BUNDLE_InitClearNeedsReset();

// Bundle Init Functions

void PROTON_BUNDLE_InitLogger()
{
  _logger_signals[PROTON_SIGNALS__LOGGER__LEVEL].signal.which_signal = proton_Signal_uint32_value_tag;
  _logger_signals[PROTON_SIGNALS__LOGGER__LEVEL].arg.data = &logger_bundle.level;

  _logger_signals[PROTON_SIGNALS__LOGGER__NAME].signal.which_signal = proton_Signal_string_value_tag;
  _logger_signals[PROTON_SIGNALS__LOGGER__NAME].signal.signal.string_value = &_logger_signals[PROTON_SIGNALS__LOGGER__NAME].arg;
  _logger_signals[PROTON_SIGNALS__LOGGER__NAME].arg.data = logger_bundle.name;
  _logger_signals[PROTON_SIGNALS__LOGGER__NAME].arg.capacity = PROTON_SIGNALS__LOGGER__NAME__CAPACITY;
  _logger_signals[PROTON_SIGNALS__LOGGER__NAME].arg.size = 0;

  _logger_signals[PROTON_SIGNALS__LOGGER__MSG].signal.which_signal = proton_Signal_string_value_tag;
  _logger_signals[PROTON_SIGNALS__LOGGER__MSG].signal.signal.string_value = &_logger_signals[PROTON_SIGNALS__LOGGER__MSG].arg;
  _logger_signals[PROTON_SIGNALS__LOGGER__MSG].arg.data = logger_bundle.msg;
  _logger_signals[PROTON_SIGNALS__LOGGER__MSG].arg.capacity = PROTON_SIGNALS__LOGGER__MSG__CAPACITY;
  _logger_signals[PROTON_SIGNALS__LOGGER__MSG].arg.size = 0;

  _logger_signals[PROTON_SIGNALS__LOGGER__FILE].signal.which_signal = proton_Signal_string_value_tag;
  _logger_signals[PROTON_SIGNALS__LOGGER__FILE].signal.signal.string_value = &_logger_signals[PROTON_SIGNALS__LOGGER__FILE].arg;
  _logger_signals[PROTON_SIGNALS__LOGGER__FILE].arg.data = logger_bundle.file;
  _logger_signals[PROTON_SIGNALS__LOGGER__FILE].arg.capacity = PROTON_SIGNALS__LOGGER__FILE__CAPACITY;
  _logger_signals[PROTON_SIGNALS__LOGGER__FILE].arg.size = 0;

  _logger_signals[PROTON_SIGNALS__LOGGER__FUNCTION].signal.which_signal = proton_Signal_string_value_tag;
  _logger_signals[PROTON_SIGNALS__LOGGER__FUNCTION].signal.signal.string_value = &_logger_signals[PROTON_SIGNALS__LOGGER__FUNCTION].arg;
  _logger_signals[PROTON_SIGNALS__LOGGER__FUNCTION].arg.data = logger_bundle.function;
  _logger_signals[PROTON_SIGNALS__LOGGER__FUNCTION].arg.capacity = PROTON_SIGNALS__LOGGER__FUNCTION__CAPACITY;
  _logger_signals[PROTON_SIGNALS__LOGGER__FUNCTION].arg.size = 0;

  _logger_signals[PROTON_SIGNALS__LOGGER__LINE].signal.which_signal = proton_Signal_uint32_value_tag;
  _logger_signals[PROTON_SIGNALS__LOGGER__LINE].arg.data = &logger_bundle.line;

  PROTON_InitBundle(&_logger_bundle, PROTON_BUNDLE__LOGGER, _logger_signals, PROTON_SIGNALS__LOGGER_COUNT);
}

void PROTON_BUNDLE_InitStatus()
{
  _status_signals[PROTON_SIGNALS__STATUS__HARDWARE_ID].signal.which_signal = proton_Signal_string_value_tag;
  _status_signals[PROTON_SIGNALS__STATUS__HARDWARE_ID].signal.signal.string_value = &_status_signals[PROTON_SIGNALS__STATUS__HARDWARE_ID].arg;
  _status_signals[PROTON_SIGNALS__STATUS__HARDWARE_ID].arg.data = status_bundle.hardware_id;
  _status_signals[PROTON_SIGNALS__STATUS__HARDWARE_ID].arg.capacity = PROTON_SIGNALS__STATUS__HARDWARE_ID__CAPACITY;
  _status_signals[PROTON_SIGNALS__STATUS__HARDWARE_ID].arg.size = 0;

  _status_signals[PROTON_SIGNALS__STATUS__FIRMWARE_VERSION].signal.which_signal = proton_Signal_string_value_tag;
  _status_signals[PROTON_SIGNALS__STATUS__FIRMWARE_VERSION].signal.signal.string_value = &_status_signals[PROTON_SIGNALS__STATUS__FIRMWARE_VERSION].arg;
  _status_signals[PROTON_SIGNALS__STATUS__FIRMWARE_VERSION].arg.data = status_bundle.firmware_version;
  _status_signals[PROTON_SIGNALS__STATUS__FIRMWARE_VERSION].arg.capacity = PROTON_SIGNALS__STATUS__FIRMWARE_VERSION__CAPACITY;
  _status_signals[PROTON_SIGNALS__STATUS__FIRMWARE_VERSION].arg.size = 0;

  _status_signals[PROTON_SIGNALS__STATUS__MCU_UPTIME_S].signal.which_signal = proton_Signal_uint32_value_tag;
  _status_signals[PROTON_SIGNALS__STATUS__MCU_UPTIME_S].arg.data = &status_bundle.mcu_uptime_s;

  _status_signals[PROTON_SIGNALS__STATUS__MCU_UPTIME_NS].signal.which_signal = proton_Signal_uint32_value_tag;
  _status_signals[PROTON_SIGNALS__STATUS__MCU_UPTIME_NS].arg.data = &status_bundle.mcu_uptime_ns;

  _status_signals[PROTON_SIGNALS__STATUS__CONNECTION_UPTIME_S].signal.which_signal = proton_Signal_uint32_value_tag;
  _status_signals[PROTON_SIGNALS__STATUS__CONNECTION_UPTIME_S].arg.data = &status_bundle.connection_uptime_s;

  _status_signals[PROTON_SIGNALS__STATUS__CONNECTION_UPTIME_NS].signal.which_signal = proton_Signal_uint32_value_tag;
  _status_signals[PROTON_SIGNALS__STATUS__CONNECTION_UPTIME_NS].arg.data = &status_bundle.connection_uptime_ns;

  PROTON_InitBundle(&_status_bundle, PROTON_BUNDLE__STATUS, _status_signals, PROTON_SIGNALS__STATUS_COUNT);
}

void PROTON_BUNDLE_InitPower()
{
  _power_signals[PROTON_SIGNALS__POWER__MEASURED_VOLTAGES].signal.which_signal = proton_Signal_list_float_value_tag;
  _power_signals[PROTON_SIGNALS__POWER__MEASURED_VOLTAGES].signal.signal.list_float_value.floats = &_power_signals[PROTON_SIGNALS__POWER__MEASURED_VOLTAGES].arg;
  _power_signals[PROTON_SIGNALS__POWER__MEASURED_VOLTAGES].arg.data = power_bundle.measured_voltages;
  _power_signals[PROTON_SIGNALS__POWER__MEASURED_VOLTAGES].arg.capacity = PROTON_SIGNALS__POWER__MEASURED_VOLTAGES__LENGTH;
  _power_signals[PROTON_SIGNALS__POWER__MEASURED_VOLTAGES].arg.size = 0;

  _power_signals[PROTON_SIGNALS__POWER__MEASURED_CURRENTS].signal.which_signal = proton_Signal_list_float_value_tag;
  _power_signals[PROTON_SIGNALS__POWER__MEASURED_CURRENTS].signal.signal.list_float_value.floats = &_power_signals[PROTON_SIGNALS__POWER__MEASURED_CURRENTS].arg;
  _power_signals[PROTON_SIGNALS__POWER__MEASURED_CURRENTS].arg.data = power_bundle.measured_currents;
  _power_signals[PROTON_SIGNALS__POWER__MEASURED_CURRENTS].arg.capacity = PROTON_SIGNALS__POWER__MEASURED_CURRENTS__LENGTH;
  _power_signals[PROTON_SIGNALS__POWER__MEASURED_CURRENTS].arg.size = 0;

  PROTON_InitBundle(&_power_bundle, PROTON_BUNDLE__POWER, _power_signals, PROTON_SIGNALS__POWER_COUNT);
}

void PROTON_BUNDLE_InitEmergencyStop()
{
  _emergency_stop_signals[PROTON_SIGNALS__EMERGENCY_STOP__STOPPED].signal.which_signal = proton_Signal_bool_value_tag;
  _emergency_stop_signals[PROTON_SIGNALS__EMERGENCY_STOP__STOPPED].arg.data = &emergency_stop_bundle.stopped;

  PROTON_InitBundle(&_emergency_stop_bundle, PROTON_BUNDLE__EMERGENCY_STOP, _emergency_stop_signals, PROTON_SIGNALS__EMERGENCY_STOP_COUNT);
}

void PROTON_BUNDLE_InitTemperature()
{
  _temperature_signals[PROTON_SIGNALS__TEMPERATURE__TEMPERATURES].signal.which_signal = proton_Signal_list_float_value_tag;
  _temperature_signals[PROTON_SIGNALS__TEMPERATURE__TEMPERATURES].signal.signal.list_float_value.floats = &_temperature_signals[PROTON_SIGNALS__TEMPERATURE__TEMPERATURES].arg;
  _temperature_signals[PROTON_SIGNALS__TEMPERATURE__TEMPERATURES].arg.data = temperature_bundle.temperatures;
  _temperature_signals[PROTON_SIGNALS__TEMPERATURE__TEMPERATURES].arg.capacity = PROTON_SIGNALS__TEMPERATURE__TEMPERATURES__LENGTH;
  _temperature_signals[PROTON_SIGNALS__TEMPERATURE__TEMPERATURES].arg.size = 0;

  PROTON_InitBundle(&_temperature_bundle, PROTON_BUNDLE__TEMPERATURE, _temperature_signals, PROTON_SIGNALS__TEMPERATURE_COUNT);
}

void PROTON_BUNDLE_InitStopStatus()
{
  _stop_status_signals[PROTON_SIGNALS__STOP_STATUS__NEEDS_RESET].signal.which_signal = proton_Signal_bool_value_tag;
  _stop_status_signals[PROTON_SIGNALS__STOP_STATUS__NEEDS_RESET].arg.data = &stop_status_bundle.needs_reset;

  PROTON_InitBundle(&_stop_status_bundle, PROTON_BUNDLE__STOP_STATUS, _stop_status_signals, PROTON_SIGNALS__STOP_STATUS_COUNT);
}

void PROTON_BUNDLE_InitPinoutState()
{
  _pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__RAILS].signal.which_signal = proton_Signal_list_bool_value_tag;
  _pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__RAILS].signal.signal.list_bool_value.bools = &_pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__RAILS].arg;
  _pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__RAILS].arg.data = pinout_state_bundle.rails;
  _pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__RAILS].arg.capacity = PROTON_SIGNALS__PINOUT_STATE__RAILS__LENGTH;
  _pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__RAILS].arg.size = 0;

  _pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__INPUTS].signal.which_signal = proton_Signal_list_bool_value_tag;
  _pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__INPUTS].signal.signal.list_bool_value.bools = &_pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__INPUTS].arg;
  _pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__INPUTS].arg.data = pinout_state_bundle.inputs;
  _pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__INPUTS].arg.capacity = PROTON_SIGNALS__PINOUT_STATE__INPUTS__LENGTH;
  _pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__INPUTS].arg.size = 0;

  _pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUTS].signal.which_signal = proton_Signal_list_bool_value_tag;
  _pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUTS].signal.signal.list_bool_value.bools = &_pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUTS].arg;
  _pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUTS].arg.data = pinout_state_bundle.outputs;
  _pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUTS].arg.capacity = PROTON_SIGNALS__PINOUT_STATE__OUTPUTS__LENGTH;
  _pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUTS].arg.size = 0;

  _pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUT_PERIODS].signal.which_signal = proton_Signal_list_uint32_value_tag;
  _pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUT_PERIODS].signal.signal.list_uint32_value.uint32s = &_pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUT_PERIODS].arg;
  _pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUT_PERIODS].arg.data = pinout_state_bundle.output_periods;
  _pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUT_PERIODS].arg.capacity = PROTON_SIGNALS__PINOUT_STATE__OUTPUT_PERIODS__LENGTH;
  _pinout_state_signals[PROTON_SIGNALS__PINOUT_STATE__OUTPUT_PERIODS].arg.size = 0;

  PROTON_InitBundle(&_pinout_state_bundle, PROTON_BUNDLE__PINOUT_STATE, _pinout_state_signals, PROTON_SIGNALS__PINOUT_STATE_COUNT);
}

void PROTON_BUNDLE_InitAlerts()
{
  _alerts_signals[PROTON_SIGNALS__ALERTS__ALERT_STRING].signal.which_signal = proton_Signal_string_value_tag;
  _alerts_signals[PROTON_SIGNALS__ALERTS__ALERT_STRING].signal.signal.string_value = &_alerts_signals[PROTON_SIGNALS__ALERTS__ALERT_STRING].arg;
  _alerts_signals[PROTON_SIGNALS__ALERTS__ALERT_STRING].arg.data = alerts_bundle.alert_string;
  _alerts_signals[PROTON_SIGNALS__ALERTS__ALERT_STRING].arg.capacity = PROTON_SIGNALS__ALERTS__ALERT_STRING__CAPACITY;
  _alerts_signals[PROTON_SIGNALS__ALERTS__ALERT_STRING].arg.size = 0;

  PROTON_InitBundle(&_alerts_bundle, PROTON_BUNDLE__ALERTS, _alerts_signals, PROTON_SIGNALS__ALERTS_COUNT);
}

void PROTON_BUNDLE_InitCmdFans()
{
  _cmd_fans_signals[PROTON_SIGNALS__CMD_FANS__FAN_SPEEDS].signal.which_signal = proton_Signal_bytes_value_tag;
  _cmd_fans_signals[PROTON_SIGNALS__CMD_FANS__FAN_SPEEDS].signal.signal.bytes_value = &_cmd_fans_signals[PROTON_SIGNALS__CMD_FANS__FAN_SPEEDS].arg;
  _cmd_fans_signals[PROTON_SIGNALS__CMD_FANS__FAN_SPEEDS].arg.data = cmd_fans_bundle.fan_speeds;
  _cmd_fans_signals[PROTON_SIGNALS__CMD_FANS__FAN_SPEEDS].arg.capacity = PROTON_SIGNALS__CMD_FANS__FAN_SPEEDS__CAPACITY;
  _cmd_fans_signals[PROTON_SIGNALS__CMD_FANS__FAN_SPEEDS].arg.size = 0;

  PROTON_InitBundle(&_cmd_fans_bundle, PROTON_BUNDLE__CMD_FANS, _cmd_fans_signals, PROTON_SIGNALS__CMD_FANS_COUNT);
}

void PROTON_BUNDLE_InitDisplayStatus()
{
  _display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_1].signal.which_signal = proton_Signal_string_value_tag;
  _display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_1].signal.signal.string_value = &_display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_1].arg;
  _display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_1].arg.data = display_status_bundle.string_1;
  _display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_1].arg.capacity = PROTON_SIGNALS__DISPLAY_STATUS__STRING_1__CAPACITY;
  _display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_1].arg.size = 0;

  _display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_2].signal.which_signal = proton_Signal_string_value_tag;
  _display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_2].signal.signal.string_value = &_display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_2].arg;
  _display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_2].arg.data = display_status_bundle.string_2;
  _display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_2].arg.capacity = PROTON_SIGNALS__DISPLAY_STATUS__STRING_2__CAPACITY;
  _display_status_signals[PROTON_SIGNALS__DISPLAY_STATUS__STRING_2].arg.size = 0;

  PROTON_InitBundle(&_display_status_bundle, PROTON_BUNDLE__DISPLAY_STATUS, _display_status_signals, PROTON_SIGNALS__DISPLAY_STATUS_COUNT);
}

void PROTON_BUNDLE_InitCmdLights()
{
  _cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_LEFT_LIGHT].signal.which_signal = proton_Signal_bytes_value_tag;
  _cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_LEFT_LIGHT].signal.signal.bytes_value = &_cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_LEFT_LIGHT].arg;
  _cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_LEFT_LIGHT].arg.data = cmd_lights_bundle.front_left_light;
  _cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_LEFT_LIGHT].arg.capacity = PROTON_SIGNALS__CMD_LIGHTS__FRONT_LEFT_LIGHT__CAPACITY;
  _cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_LEFT_LIGHT].arg.size = 0;

  _cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_RIGHT_LIGHT].signal.which_signal = proton_Signal_bytes_value_tag;
  _cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_RIGHT_LIGHT].signal.signal.bytes_value = &_cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_RIGHT_LIGHT].arg;
  _cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_RIGHT_LIGHT].arg.data = cmd_lights_bundle.front_right_light;
  _cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_RIGHT_LIGHT].arg.capacity = PROTON_SIGNALS__CMD_LIGHTS__FRONT_RIGHT_LIGHT__CAPACITY;
  _cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__FRONT_RIGHT_LIGHT].arg.size = 0;

  _cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_LEFT_LIGHT].signal.which_signal = proton_Signal_bytes_value_tag;
  _cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_LEFT_LIGHT].signal.signal.bytes_value = &_cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_LEFT_LIGHT].arg;
  _cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_LEFT_LIGHT].arg.data = cmd_lights_bundle.rear_left_light;
  _cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_LEFT_LIGHT].arg.capacity = PROTON_SIGNALS__CMD_LIGHTS__REAR_LEFT_LIGHT__CAPACITY;
  _cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_LEFT_LIGHT].arg.size = 0;

  _cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_RIGHT_LIGHT].signal.which_signal = proton_Signal_bytes_value_tag;
  _cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_RIGHT_LIGHT].signal.signal.bytes_value = &_cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_RIGHT_LIGHT].arg;
  _cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_RIGHT_LIGHT].arg.data = cmd_lights_bundle.rear_right_light;
  _cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_RIGHT_LIGHT].arg.capacity = PROTON_SIGNALS__CMD_LIGHTS__REAR_RIGHT_LIGHT__CAPACITY;
  _cmd_lights_signals[PROTON_SIGNALS__CMD_LIGHTS__REAR_RIGHT_LIGHT].arg.size = 0;

  PROTON_InitBundle(&_cmd_lights_bundle, PROTON_BUNDLE__CMD_LIGHTS, _cmd_lights_signals, PROTON_SIGNALS__CMD_LIGHTS_COUNT);
}

void PROTON_BUNDLE_InitBattery()
{
  _battery_signals[PROTON_SIGNALS__BATTERY__PERCENTAGE].signal.which_signal = proton_Signal_float_value_tag;
  _battery_signals[PROTON_SIGNALS__BATTERY__PERCENTAGE].arg.data = &battery_bundle.percentage;

  PROTON_InitBundle(&_battery_bundle, PROTON_BUNDLE__BATTERY, _battery_signals, PROTON_SIGNALS__BATTERY_COUNT);
}

void PROTON_BUNDLE_InitPinoutCommand()
{
  _pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__RAILS].signal.which_signal = proton_Signal_list_bool_value_tag;
  _pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__RAILS].signal.signal.list_bool_value.bools = &_pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__RAILS].arg;
  _pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__RAILS].arg.data = pinout_command_bundle.rails;
  _pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__RAILS].arg.capacity = PROTON_SIGNALS__PINOUT_COMMAND__RAILS__LENGTH;
  _pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__RAILS].arg.size = 0;

  _pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__OUTPUTS].signal.which_signal = proton_Signal_list_uint32_value_tag;
  _pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__OUTPUTS].signal.signal.list_uint32_value.uint32s = &_pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__OUTPUTS].arg;
  _pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__OUTPUTS].arg.data = pinout_command_bundle.outputs;
  _pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__OUTPUTS].arg.capacity = PROTON_SIGNALS__PINOUT_COMMAND__OUTPUTS__LENGTH;
  _pinout_command_signals[PROTON_SIGNALS__PINOUT_COMMAND__OUTPUTS].arg.size = 0;

  PROTON_InitBundle(&_pinout_command_bundle, PROTON_BUNDLE__PINOUT_COMMAND, _pinout_command_signals, PROTON_SIGNALS__PINOUT_COMMAND_COUNT);
}

void PROTON_BUNDLE_InitCmdShutdown()
{
  PROTON_InitBundle(&_cmd_shutdown_bundle, PROTON_BUNDLE__CMD_SHUTDOWN, _cmd_shutdown_signals, PROTON_SIGNALS__CMD_SHUTDOWN_COUNT);
}

void PROTON_BUNDLE_InitClearNeedsReset()
{
  PROTON_InitBundle(&_clear_needs_reset_bundle, PROTON_BUNDLE__CLEAR_NEEDS_RESET, _clear_needs_reset_signals, PROTON_SIGNALS__CLEAR_NEEDS_RESET_COUNT);
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
  PROTON_BUNDLE_InitCmdShutdown();
  PROTON_BUNDLE_InitClearNeedsReset();
}

// Bundle Receive Function

bool PROTON_BUNDLE_Receive(const uint8_t* buffer, size_t length)
{
  proton_bundle_t * bundle;
  PROTON_BUNDLE_e id;
  proton_callback_t callback;

  // Decode bundle ID
  if (!PROTON_DecodeId(&id, buffer, length))
  {
    return false;
  }

  switch (id)
  {
    case PROTON_BUNDLE__CMD_FANS:
    {
      bundle = &_cmd_fans_bundle;
      callback = PROTON_BUNDLE_CmdFansCallback;
      break;
    }

    case PROTON_BUNDLE__DISPLAY_STATUS:
    {
      bundle = &_display_status_bundle;
      callback = PROTON_BUNDLE_DisplayStatusCallback;
      break;
    }

    case PROTON_BUNDLE__CMD_LIGHTS:
    {
      bundle = &_cmd_lights_bundle;
      callback = PROTON_BUNDLE_CmdLightsCallback;
      break;
    }

    case PROTON_BUNDLE__BATTERY:
    {
      bundle = &_battery_bundle;
      callback = PROTON_BUNDLE_BatteryCallback;
      break;
    }

    case PROTON_BUNDLE__PINOUT_COMMAND:
    {
      bundle = &_pinout_command_bundle;
      callback = PROTON_BUNDLE_PinoutCommandCallback;
      break;
    }

    case PROTON_BUNDLE__CMD_SHUTDOWN:
    {
      bundle = &_cmd_shutdown_bundle;
      callback = PROTON_BUNDLE_CmdShutdownCallback;
      break;
    }

    case PROTON_BUNDLE__CLEAR_NEEDS_RESET:
    {
      bundle = &_clear_needs_reset_bundle;
      callback = PROTON_BUNDLE_ClearNeedsResetCallback;
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

// Bundle Send Function

bool PROTON_BUNDLE_Send(PROTON_BUNDLE_e bundle)
{
  proton_bundle_t * bundle_;

  switch (bundle)
  {
    case PROTON_BUNDLE__LOGGER:
    {
      bundle_= &_logger_bundle;
      break;
    }

    case PROTON_BUNDLE__STATUS:
    {
      bundle_= &_status_bundle;
      break;
    }

    case PROTON_BUNDLE__POWER:
    {
      bundle_= &_power_bundle;
      break;
    }

    case PROTON_BUNDLE__EMERGENCY_STOP:
    {
      bundle_= &_emergency_stop_bundle;
      break;
    }

    case PROTON_BUNDLE__TEMPERATURE:
    {
      bundle_= &_temperature_bundle;
      break;
    }

    case PROTON_BUNDLE__STOP_STATUS:
    {
      bundle_= &_stop_status_bundle;
      break;
    }

    case PROTON_BUNDLE__PINOUT_STATE:
    {
      bundle_= &_pinout_state_bundle;
      break;
    }

    case PROTON_BUNDLE__ALERTS:
    {
      bundle_= &_alerts_bundle;
      break;
    }

    default:
    {
      return false;
    }
  }

  // Encode bundle
  bool ret = false;

  if (PROTON_MUTEX__McuLock())
  {
    int bytes_written = PROTON_Encode(bundle_, mcu_node.write_buf.data, mcu_node.write_buf.len);
    if (bytes_written > 0 && mcu_node.connected && mcu_node.transport.write)
    {
      // Send bundle
      ret = mcu_node.transport.write(mcu_node.write_buf.data, bytes_written) > 0;
    }

    PROTON_MUTEX__McuUnlock();
  }

  return ret;
}

// Proton Init

void PROTON_Init()
{
  proton_transport_t mcu_transport;
  mcu_transport.connect = PROTON_TRANSPORT__McuConnect;
  mcu_transport.disconnect = PROTON_TRANSPORT__McuDisconnect;
  mcu_transport.read = PROTON_TRANSPORT__McuRead;
  mcu_transport.write = PROTON_TRANSPORT__McuWrite;

  PROTON_BUNDLE_Init();

  PROTON_InitNode(&mcu_node, mcu_transport, PROTON_BUNDLE_Receive, proton_mcu_read_buffer, proton_mcu_write_buffer);
}

