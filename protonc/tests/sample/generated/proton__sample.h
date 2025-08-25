
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

typedef enum PROTON_SIGNALS__test {
  PROTON_SIGNALS__TEST__DOUBLE_MESSAGE,
  PROTON_SIGNALS__TEST__FLOAT_MESSAGE,
  PROTON_SIGNALS__TEST__INT32_MESSAGE,
  PROTON_SIGNALS__TEST__INT64_MESSAGE,
  PROTON_SIGNALS__TEST__UINT32_MESSAGE,
  PROTON_SIGNALS__TEST__UINT64_MESSAGE,
  PROTON_SIGNALS__TEST__BOOL_MESSAGE,
  PROTON_SIGNALS__TEST__STRING_MESSAGE,
  PROTON_SIGNALS__TEST__BYTES_MESSAGE,
  PROTON_SIGNALS__TEST__DOUBLE_LIST,
  PROTON_SIGNALS__TEST__FLOAT_LIST,
  PROTON_SIGNALS__TEST__INT32_LIST,
  PROTON_SIGNALS__TEST__INT64_LIST,
  PROTON_SIGNALS__TEST__UINT32_LIST,
  PROTON_SIGNALS__TEST__UINT64_LIST,
  PROTON_SIGNALS__TEST__BOOL_LIST,
  PROTON_SIGNALS__TEST__STRING_LIST,
  PROTON_SIGNALS__TEST_COUNT
} PROTON_SIGNALS__test_e;


// Message Structure Definitions

typedef struct PROTON_BUNDLE__test {
  double double_message;
  float float_message;
  int32_t int32_message;
  int64_t int64_message;
  uint32_t uint32_message;
  uint64_t uint64_message;
  bool bool_message;
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
} PROTON_BUNDLE__test_t;

// External Message Structures

extern PROTON_BUNDLE__test_t test_struct;

// External Bundles

extern proton_bundle_t test_bundle;

// Message Init Prototypes

void PROTON_MESSAGE_init();

#endif  // PROTONC__PROTON__SAMPLE_H
