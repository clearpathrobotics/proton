
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

#include "proton__sample.h"

// Message Structures

PROTON_MESSAGE_STRUCT__imu_t imu;
PROTON_MESSAGE_STRUCT__lights_t lights;
PROTON_MESSAGE_STRUCT__joints_t joints;
PROTON_MESSAGE_STRUCT__test_proton_t test_proton;
PROTON_MESSAGE_STRUCT__needs_reset_button_t needs_reset_button;
PROTON_MESSAGE_STRUCT__get_firmware_version_t get_firmware_version;
PROTON_MESSAGE_STRUCT__get_firmware_version_response_t get_firmware_version_response;

// Signals

proton_Signal imu_signals[PROTON_SIGNAL__IMU_COUNT];
proton_Signal lights_signals[PROTON_SIGNAL__LIGHTS_COUNT];
proton_Signal joints_signals[PROTON_SIGNAL__JOINTS_COUNT];
proton_Signal test_proton_signals[PROTON_SIGNAL__TEST_PROTON_COUNT];
proton_Signal needs_reset_button_signals[PROTON_SIGNAL__NEEDS_RESET_BUTTON_COUNT];
proton_Signal get_firmware_version_signals[PROTON_SIGNAL__GET_FIRMWARE_VERSION_COUNT];
proton_Signal get_firmware_version_response_signals[PROTON_SIGNAL__GET_FIRMWARE_VERSION_RESPONSE_COUNT];

// Signal schemas

proton_signal_schema_t imu_signal_schema[PROTON_SIGNAL__IMU_COUNT];
proton_signal_schema_t lights_signal_schema[PROTON_SIGNAL__LIGHTS_COUNT];
proton_signal_schema_t joints_signal_schema[PROTON_SIGNAL__JOINTS_COUNT];
proton_signal_schema_t test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON_COUNT];
proton_signal_schema_t needs_reset_button_signal_schema[PROTON_SIGNAL__NEEDS_RESET_BUTTON_COUNT];
proton_signal_schema_t get_firmware_version_signal_schema[PROTON_SIGNAL__GET_FIRMWARE_VERSION_COUNT];
proton_signal_schema_t get_firmware_version_response_signal_schema[PROTON_SIGNAL__GET_FIRMWARE_VERSION_RESPONSE_COUNT];

// Protons

proton_t imu_proton;
proton_t lights_proton;
proton_t joints_proton;
proton_t test_proton_proton;
proton_t needs_reset_button_proton;
proton_t get_firmware_version_proton;
proton_t get_firmware_version_response_proton;

// Message Init Prototypes

void PROTON_MESSAGE_init_imu();
void PROTON_MESSAGE_init_lights();
void PROTON_MESSAGE_init_joints();
void PROTON_MESSAGE_init_test_proton();
void PROTON_MESSAGE_init_needs_reset_button();
void PROTON_MESSAGE_init_get_firmware_version();
void PROTON_MESSAGE_init_get_firmware_version_response();

// Message Init Functions

void PROTON_MESSAGE_init_imu()
{
  imu_signal_schema[PROTON_SIGNAL__IMU__LINEAR_ACCEL_X].type = PROTON_SIGNAL_TYPE_DOUBLE_VALUE;
  imu_signal_schema[PROTON_SIGNAL__IMU__LINEAR_ACCEL_Y].type = PROTON_SIGNAL_TYPE_DOUBLE_VALUE;
  imu_signal_schema[PROTON_SIGNAL__IMU__LINEAR_ACCEL_Z].type = PROTON_SIGNAL_TYPE_DOUBLE_VALUE;
  imu_signal_schema[PROTON_SIGNAL__IMU__ANGULAR_VELOCITY_X].type = PROTON_SIGNAL_TYPE_DOUBLE_VALUE;
  imu_signal_schema[PROTON_SIGNAL__IMU__ANGULAR_VELOCITY_Y].type = PROTON_SIGNAL_TYPE_DOUBLE_VALUE;
  imu_signal_schema[PROTON_SIGNAL__IMU__ANGULAR_VELOCITY_Z].type = PROTON_SIGNAL_TYPE_DOUBLE_VALUE;
  PROTON_InitProton(&imu_proton, 256, imu_signals, imu_signal_schema, PROTON_SIGNAL__IMU_COUNT);
}

void PROTON_MESSAGE_init_lights()
{
  lights_signal_schema[PROTON_SIGNAL__LIGHTS__FRONT_LEFT_LIGHT].type = PROTON_SIGNAL_TYPE_LIST_UINT32_VALUE;
  lights_signal_schema[PROTON_SIGNAL__LIGHTS__FRONT_LEFT_LIGHT].arg.values = lights.front_left_light;
  lights_signal_schema[PROTON_SIGNAL__LIGHTS__FRONT_LEFT_LIGHT].arg.capacity = 3;
  lights_signal_schema[PROTON_SIGNAL__LIGHTS__FRONT_LEFT_LIGHT].arg.size = 0;
  lights_signal_schema[PROTON_SIGNAL__LIGHTS__FRONT_RIGHT_LIGHT].type = PROTON_SIGNAL_TYPE_LIST_UINT32_VALUE;
  lights_signal_schema[PROTON_SIGNAL__LIGHTS__FRONT_RIGHT_LIGHT].arg.values = lights.front_right_light;
  lights_signal_schema[PROTON_SIGNAL__LIGHTS__FRONT_RIGHT_LIGHT].arg.capacity = 3;
  lights_signal_schema[PROTON_SIGNAL__LIGHTS__FRONT_RIGHT_LIGHT].arg.size = 0;
  lights_signal_schema[PROTON_SIGNAL__LIGHTS__REAR_LEFT_LIGHT].type = PROTON_SIGNAL_TYPE_LIST_UINT32_VALUE;
  lights_signal_schema[PROTON_SIGNAL__LIGHTS__REAR_LEFT_LIGHT].arg.values = lights.rear_left_light;
  lights_signal_schema[PROTON_SIGNAL__LIGHTS__REAR_LEFT_LIGHT].arg.capacity = 3;
  lights_signal_schema[PROTON_SIGNAL__LIGHTS__REAR_LEFT_LIGHT].arg.size = 0;
  lights_signal_schema[PROTON_SIGNAL__LIGHTS__REAR_RIGHT_LIGHT].type = PROTON_SIGNAL_TYPE_LIST_UINT32_VALUE;
  lights_signal_schema[PROTON_SIGNAL__LIGHTS__REAR_RIGHT_LIGHT].arg.values = lights.rear_right_light;
  lights_signal_schema[PROTON_SIGNAL__LIGHTS__REAR_RIGHT_LIGHT].arg.capacity = 3;
  lights_signal_schema[PROTON_SIGNAL__LIGHTS__REAR_RIGHT_LIGHT].arg.size = 0;
  PROTON_InitProton(&lights_proton, 512, lights_signals, lights_signal_schema, PROTON_SIGNAL__LIGHTS_COUNT);
}

void PROTON_MESSAGE_init_joints()
{
  joints_signal_schema[PROTON_SIGNAL__JOINTS__JOINT_STATES].type = PROTON_SIGNAL_TYPE_LIST_STRING_VALUE;
  joints_signal_schema[PROTON_SIGNAL__JOINTS__JOINT_STATES].arg.values = joints.joint_states.list;
  joints_signal_schema[PROTON_SIGNAL__JOINTS__JOINT_STATES].arg.capacity = 3;
  joints_signal_schema[PROTON_SIGNAL__JOINTS__JOINT_STATES].arg.size = 0;
  for (int i = 0; i < 3; i += 1)
  {
    joints.joint_states.list[i] = joints.joint_states.strings[i];
  }
  PROTON_InitProton(&joints_proton, 1280, joints_signals, joints_signal_schema, PROTON_SIGNAL__JOINTS_COUNT);
}

void PROTON_MESSAGE_init_test_proton()
{
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__STRING_MESSAGE].type = PROTON_SIGNAL_TYPE_STRING_VALUE;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__STRING_MESSAGE].arg.values = test_proton.string_message;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__STRING_MESSAGE].arg.capacity = 64;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__STRING_MESSAGE].arg.size = 0;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__BYTES_MESSAGE].type = PROTON_SIGNAL_TYPE_BYTES_VALUE;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__BYTES_MESSAGE].arg.values = test_proton.bytes_message;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__BYTES_MESSAGE].arg.capacity = 64;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__BYTES_MESSAGE].arg.size = 0;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__DOUBLE_LIST].type = PROTON_SIGNAL_TYPE_LIST_DOUBLE_VALUE;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__DOUBLE_LIST].arg.values = test_proton.double_list;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__DOUBLE_LIST].arg.capacity = 10;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__DOUBLE_LIST].arg.size = 0;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__FLOAT_LIST].type = PROTON_SIGNAL_TYPE_LIST_FLOAT_VALUE;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__FLOAT_LIST].arg.values = test_proton.float_list;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__FLOAT_LIST].arg.capacity = 10;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__FLOAT_LIST].arg.size = 0;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__INT32_LIST].type = PROTON_SIGNAL_TYPE_LIST_INT32_VALUE;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__INT32_LIST].arg.values = test_proton.int32_list;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__INT32_LIST].arg.capacity = 10;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__INT32_LIST].arg.size = 0;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__INT64_LIST].type = PROTON_SIGNAL_TYPE_LIST_INT64_VALUE;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__INT64_LIST].arg.values = test_proton.int64_list;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__INT64_LIST].arg.capacity = 10;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__INT64_LIST].arg.size = 0;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__UINT32_LIST].type = PROTON_SIGNAL_TYPE_LIST_UINT32_VALUE;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__UINT32_LIST].arg.values = test_proton.uint32_list;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__UINT32_LIST].arg.capacity = 10;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__UINT32_LIST].arg.size = 0;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__UINT64_LIST].type = PROTON_SIGNAL_TYPE_LIST_UINT64_VALUE;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__UINT64_LIST].arg.values = test_proton.uint64_list;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__UINT64_LIST].arg.capacity = 10;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__UINT64_LIST].arg.size = 0;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__BOOL_LIST].type = PROTON_SIGNAL_TYPE_LIST_BOOL_VALUE;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__BOOL_LIST].arg.values = test_proton.bool_list;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__BOOL_LIST].arg.capacity = 10;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__BOOL_LIST].arg.size = 0;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__STRING_LIST].type = PROTON_SIGNAL_TYPE_LIST_STRING_VALUE;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__STRING_LIST].arg.values = test_proton.string_list.list;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__STRING_LIST].arg.capacity = 10;
  test_proton_signal_schema[PROTON_SIGNAL__TEST_PROTON__STRING_LIST].arg.size = 0;
  for (int i = 0; i < 10; i += 1)
  {
    test_proton.string_list.list[i] = test_proton.string_list.strings[i];
  }
  PROTON_InitProton(&test_proton_proton, 4660, test_proton_signals, test_proton_signal_schema, PROTON_SIGNAL__TEST_PROTON_COUNT);
}

void PROTON_MESSAGE_init_needs_reset_button()
{
  needs_reset_button_signal_schema[PROTON_SIGNAL__NEEDS_RESET_BUTTON__NEEDS_RESET_BUTTON_STATE].type = PROTON_SIGNAL_TYPE_BOOL_VALUE;
  PROTON_InitProton(&needs_reset_button_proton, 768, needs_reset_button_signals, needs_reset_button_signal_schema, PROTON_SIGNAL__NEEDS_RESET_BUTTON_COUNT);
}

void PROTON_MESSAGE_init_get_firmware_version()
{
  get_firmware_version_signal_schema[PROTON_SIGNAL__GET_FIRMWARE_VERSION__GET_FIRMWARE_VERSION].type = PROTON_SIGNAL_TYPE_BOOL_VALUE;
  PROTON_InitProton(&get_firmware_version_proton, 1, get_firmware_version_signals, get_firmware_version_signal_schema, PROTON_SIGNAL__GET_FIRMWARE_VERSION_COUNT);
}

void PROTON_MESSAGE_init_get_firmware_version_response()
{
  get_firmware_version_response_signal_schema[PROTON_SIGNAL__GET_FIRMWARE_VERSION_RESPONSE__FIRMWARE_VERSION].type = PROTON_SIGNAL_TYPE_STRING_VALUE;
  get_firmware_version_response_signal_schema[PROTON_SIGNAL__GET_FIRMWARE_VERSION_RESPONSE__FIRMWARE_VERSION].arg.values = get_firmware_version_response.firmware_version;
  get_firmware_version_response_signal_schema[PROTON_SIGNAL__GET_FIRMWARE_VERSION_RESPONSE__FIRMWARE_VERSION].arg.capacity = 64;
  get_firmware_version_response_signal_schema[PROTON_SIGNAL__GET_FIRMWARE_VERSION_RESPONSE__FIRMWARE_VERSION].arg.size = 0;
  PROTON_InitProton(&get_firmware_version_response_proton, 1, get_firmware_version_response_signals, get_firmware_version_response_signal_schema, PROTON_SIGNAL__GET_FIRMWARE_VERSION_RESPONSE_COUNT);
}

void PROTON_MESSAGE_init()
{
  PROTON_MESSAGE_init_imu();
  PROTON_MESSAGE_init_lights();
  PROTON_MESSAGE_init_joints();
  PROTON_MESSAGE_init_test_proton();
  PROTON_MESSAGE_init_needs_reset_button();
  PROTON_MESSAGE_init_get_firmware_version();
  PROTON_MESSAGE_init_get_firmware_version_response();
}

