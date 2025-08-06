
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

#ifndef PROTONC__PROTON__SAMPLE_H
#define PROTONC__PROTON__SAMPLE_H

#include "stdint.h"
#include "stdbool.h"
#include "proton.h"

// Signal Enums

typedef enum PROTON_SIGNAL__imu {
  PROTON_SIGNAL__IMU__LINEAR_ACCEL_X,
  PROTON_SIGNAL__IMU__LINEAR_ACCEL_Y,
  PROTON_SIGNAL__IMU__LINEAR_ACCEL_Z,
  PROTON_SIGNAL__IMU__ANGULAR_VELOCITY_X,
  PROTON_SIGNAL__IMU__ANGULAR_VELOCITY_Y,
  PROTON_SIGNAL__IMU__ANGULAR_VELOCITY_Z,
  PROTON_SIGNAL__IMU_COUNT
} PROTON_SIGNAL__imu_e;

typedef enum PROTON_SIGNAL__lights {
  PROTON_SIGNAL__LIGHTS__FRONT_LEFT_LIGHT,
  PROTON_SIGNAL__LIGHTS__FRONT_RIGHT_LIGHT,
  PROTON_SIGNAL__LIGHTS__REAR_LEFT_LIGHT,
  PROTON_SIGNAL__LIGHTS__REAR_RIGHT_LIGHT,
  PROTON_SIGNAL__LIGHTS_COUNT
} PROTON_SIGNAL__lights_e;

typedef enum PROTON_SIGNAL__joints {
  PROTON_SIGNAL__JOINTS__JOINT_STATES,
  PROTON_SIGNAL__JOINTS_COUNT
} PROTON_SIGNAL__joints_e;

typedef enum PROTON_SIGNAL__test_proton {
  PROTON_SIGNAL__TEST_PROTON__STRING_MESSAGE,
  PROTON_SIGNAL__TEST_PROTON__BYTES_MESSAGE,
  PROTON_SIGNAL__TEST_PROTON__DOUBLE_LIST,
  PROTON_SIGNAL__TEST_PROTON__FLOAT_LIST,
  PROTON_SIGNAL__TEST_PROTON__INT32_LIST,
  PROTON_SIGNAL__TEST_PROTON__INT64_LIST,
  PROTON_SIGNAL__TEST_PROTON__UINT32_LIST,
  PROTON_SIGNAL__TEST_PROTON__UINT64_LIST,
  PROTON_SIGNAL__TEST_PROTON__BOOL_LIST,
  PROTON_SIGNAL__TEST_PROTON__STRING_LIST,
  PROTON_SIGNAL__TEST_PROTON_COUNT
} PROTON_SIGNAL__test_proton_e;

typedef enum PROTON_SIGNAL__needs_reset_button {
  PROTON_SIGNAL__NEEDS_RESET_BUTTON__NEEDS_RESET_BUTTON_STATE,
  PROTON_SIGNAL__NEEDS_RESET_BUTTON_COUNT
} PROTON_SIGNAL__needs_reset_button_e;

typedef enum PROTON_SIGNAL__get_firmware_version {
  PROTON_SIGNAL__GET_FIRMWARE_VERSION__GET_FIRMWARE_VERSION,
  PROTON_SIGNAL__GET_FIRMWARE_VERSION_COUNT
} PROTON_SIGNAL__get_firmware_version_e;

typedef enum PROTON_SIGNAL__get_firmware_version_response {
  PROTON_SIGNAL__GET_FIRMWARE_VERSION_RESPONSE__FIRMWARE_VERSION,
  PROTON_SIGNAL__GET_FIRMWARE_VERSION_RESPONSE_COUNT
} PROTON_SIGNAL__get_firmware_version_response_e;


// Message Structure Definitions

typedef struct PROTON_MESSAGE_STRUCT__imu {
  double linear_accel_x;
  double linear_accel_y;
  double linear_accel_z;
  double angular_velocity_x;
  double angular_velocity_y;
  double angular_velocity_z;
} PROTON_MESSAGE_STRUCT__imu_t;

typedef struct PROTON_MESSAGE_STRUCT__lights {
  uint32_t front_left_light[3];
  uint32_t front_right_light[3];
  uint32_t rear_left_light[3];
  uint32_t rear_right_light[3];
} PROTON_MESSAGE_STRUCT__lights_t;

typedef struct PROTON_MESSAGE_STRUCT__joints {
  struct {
    char * list[3];
    char strings[3][64];
  } joint_states;
} PROTON_MESSAGE_STRUCT__joints_t;

typedef struct PROTON_MESSAGE_STRUCT__test_proton {
  char string_message[64];
  uint8_t bytes_message[64];
  double double_list[10];
  float float_list[10];
  int32_t int32_list[10];
  int64_t int64_list[10];
  uint32_t uint32_list[10];
  uint64_t uint64_list[10];
  bool bool_list[10];
  struct {
    char * list[10];
    char strings[10][64];
  } string_list;
} PROTON_MESSAGE_STRUCT__test_proton_t;

typedef struct PROTON_MESSAGE_STRUCT__needs_reset_button {
  bool needs_reset_button_state;
} PROTON_MESSAGE_STRUCT__needs_reset_button_t;

typedef struct PROTON_MESSAGE_STRUCT__get_firmware_version {
  bool get_firmware_version;
} PROTON_MESSAGE_STRUCT__get_firmware_version_t;

typedef struct PROTON_MESSAGE_STRUCT__get_firmware_version_response {
  char firmware_version[64];
} PROTON_MESSAGE_STRUCT__get_firmware_version_response_t;

// External Message Structures

extern PROTON_MESSAGE_STRUCT__imu_t imu;
extern PROTON_MESSAGE_STRUCT__lights_t lights;
extern PROTON_MESSAGE_STRUCT__joints_t joints;
extern PROTON_MESSAGE_STRUCT__test_proton_t test_proton;
extern PROTON_MESSAGE_STRUCT__needs_reset_button_t needs_reset_button;
extern PROTON_MESSAGE_STRUCT__get_firmware_version_t get_firmware_version;
extern PROTON_MESSAGE_STRUCT__get_firmware_version_response_t get_firmware_version_response;

// External Protons

extern proton_t imu_proton;
extern proton_t lights_proton;
extern proton_t joints_proton;
extern proton_t test_proton_proton;
extern proton_t needs_reset_button_proton;
extern proton_t get_firmware_version_proton;
extern proton_t get_firmware_version_response_proton;

// Message Init Prototypes

void PROTON_MESSAGE_init();

#endif  // PROTONC__PROTON__SAMPLE_H
