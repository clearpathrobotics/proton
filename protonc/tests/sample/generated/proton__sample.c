
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

PROTON_BUNDLE__test_t test_struct;

// Signals

proton_signal_t test_signals[PROTON_SIGNALS__TEST_COUNT];

// Bundles

proton_bundle_t test_bundle;

// Message Init Prototypes

void PROTON_BUNDLE_init_test();

// Message Init Functions

void PROTON_BUNDLE_init_test()
{
  test_signals[PROTON_SIGNALS__TEST__DOUBLE_MESSAGE].signal.which_signal = proton_Signal_double_value_tag;
  test_signals[PROTON_SIGNALS__TEST__DOUBLE_MESSAGE].arg.data = &test_struct.double_message;

  test_signals[PROTON_SIGNALS__TEST__FLOAT_MESSAGE].signal.which_signal = proton_Signal_float_value_tag;
  test_signals[PROTON_SIGNALS__TEST__FLOAT_MESSAGE].arg.data = &test_struct.float_message;

  test_signals[PROTON_SIGNALS__TEST__INT32_MESSAGE].signal.which_signal = proton_Signal_int32_value_tag;
  test_signals[PROTON_SIGNALS__TEST__INT32_MESSAGE].arg.data = &test_struct.int32_message;

  test_signals[PROTON_SIGNALS__TEST__INT64_MESSAGE].signal.which_signal = proton_Signal_int64_value_tag;
  test_signals[PROTON_SIGNALS__TEST__INT64_MESSAGE].arg.data = &test_struct.int64_message;

  test_signals[PROTON_SIGNALS__TEST__UINT32_MESSAGE].signal.which_signal = proton_Signal_uint32_value_tag;
  test_signals[PROTON_SIGNALS__TEST__UINT32_MESSAGE].arg.data = &test_struct.uint32_message;

  test_signals[PROTON_SIGNALS__TEST__UINT64_MESSAGE].signal.which_signal = proton_Signal_uint64_value_tag;
  test_signals[PROTON_SIGNALS__TEST__UINT64_MESSAGE].arg.data = &test_struct.uint64_message;

  test_signals[PROTON_SIGNALS__TEST__BOOL_MESSAGE].signal.which_signal = proton_Signal_bool_value_tag;
  test_signals[PROTON_SIGNALS__TEST__BOOL_MESSAGE].arg.data = &test_struct.bool_message;

  test_signals[PROTON_SIGNALS__TEST__STRING_MESSAGE].signal.which_signal = proton_Signal_string_value_tag;
  test_signals[PROTON_SIGNALS__TEST__STRING_MESSAGE].signal.signal.string_value = &test_signals[PROTON_SIGNALS__TEST__STRING_MESSAGE].arg;
  test_signals[PROTON_SIGNALS__TEST__STRING_MESSAGE].arg.data = test_struct.string_message;
  test_signals[PROTON_SIGNALS__TEST__STRING_MESSAGE].arg.capacity = 64;
  test_signals[PROTON_SIGNALS__TEST__STRING_MESSAGE].arg.size = 0;

  test_signals[PROTON_SIGNALS__TEST__BYTES_MESSAGE].signal.which_signal = proton_Signal_bytes_value_tag;
  test_signals[PROTON_SIGNALS__TEST__BYTES_MESSAGE].signal.signal.bytes_value = &test_signals[PROTON_SIGNALS__TEST__BYTES_MESSAGE].arg;
  test_signals[PROTON_SIGNALS__TEST__BYTES_MESSAGE].arg.data = test_struct.bytes_message;
  test_signals[PROTON_SIGNALS__TEST__BYTES_MESSAGE].arg.capacity = 64;
  test_signals[PROTON_SIGNALS__TEST__BYTES_MESSAGE].arg.size = 0;

  test_signals[PROTON_SIGNALS__TEST__DOUBLE_LIST].signal.which_signal = proton_Signal_list_double_value_tag;
  test_signals[PROTON_SIGNALS__TEST__DOUBLE_LIST].signal.signal.list_double_value.doubles = &test_signals[PROTON_SIGNALS__TEST__DOUBLE_LIST].arg;
  test_signals[PROTON_SIGNALS__TEST__DOUBLE_LIST].arg.data = test_struct.double_list;
  test_signals[PROTON_SIGNALS__TEST__DOUBLE_LIST].arg.capacity = 10;
  test_signals[PROTON_SIGNALS__TEST__DOUBLE_LIST].arg.size = 0;

  test_signals[PROTON_SIGNALS__TEST__FLOAT_LIST].signal.which_signal = proton_Signal_list_float_value_tag;
  test_signals[PROTON_SIGNALS__TEST__FLOAT_LIST].signal.signal.list_float_value.floats = &test_signals[PROTON_SIGNALS__TEST__FLOAT_LIST].arg;
  test_signals[PROTON_SIGNALS__TEST__FLOAT_LIST].arg.data = test_struct.float_list;
  test_signals[PROTON_SIGNALS__TEST__FLOAT_LIST].arg.capacity = 10;
  test_signals[PROTON_SIGNALS__TEST__FLOAT_LIST].arg.size = 0;

  test_signals[PROTON_SIGNALS__TEST__INT32_LIST].signal.which_signal = proton_Signal_list_int32_value_tag;
  test_signals[PROTON_SIGNALS__TEST__INT32_LIST].signal.signal.list_int32_value.int32s = &test_signals[PROTON_SIGNALS__TEST__INT32_LIST].arg;
  test_signals[PROTON_SIGNALS__TEST__INT32_LIST].arg.data = test_struct.int32_list;
  test_signals[PROTON_SIGNALS__TEST__INT32_LIST].arg.capacity = 10;
  test_signals[PROTON_SIGNALS__TEST__INT32_LIST].arg.size = 0;

  test_signals[PROTON_SIGNALS__TEST__INT64_LIST].signal.which_signal = proton_Signal_list_int64_value_tag;
  test_signals[PROTON_SIGNALS__TEST__INT64_LIST].signal.signal.list_int64_value.int64s = &test_signals[PROTON_SIGNALS__TEST__INT64_LIST].arg;
  test_signals[PROTON_SIGNALS__TEST__INT64_LIST].arg.data = test_struct.int64_list;
  test_signals[PROTON_SIGNALS__TEST__INT64_LIST].arg.capacity = 10;
  test_signals[PROTON_SIGNALS__TEST__INT64_LIST].arg.size = 0;

  test_signals[PROTON_SIGNALS__TEST__UINT32_LIST].signal.which_signal = proton_Signal_list_uint32_value_tag;
  test_signals[PROTON_SIGNALS__TEST__UINT32_LIST].signal.signal.list_uint32_value.uint32s = &test_signals[PROTON_SIGNALS__TEST__UINT32_LIST].arg;
  test_signals[PROTON_SIGNALS__TEST__UINT32_LIST].arg.data = test_struct.uint32_list;
  test_signals[PROTON_SIGNALS__TEST__UINT32_LIST].arg.capacity = 10;
  test_signals[PROTON_SIGNALS__TEST__UINT32_LIST].arg.size = 0;

  test_signals[PROTON_SIGNALS__TEST__UINT64_LIST].signal.which_signal = proton_Signal_list_uint64_value_tag;
  test_signals[PROTON_SIGNALS__TEST__UINT64_LIST].signal.signal.list_uint64_value.uint64s = &test_signals[PROTON_SIGNALS__TEST__UINT64_LIST].arg;
  test_signals[PROTON_SIGNALS__TEST__UINT64_LIST].arg.data = test_struct.uint64_list;
  test_signals[PROTON_SIGNALS__TEST__UINT64_LIST].arg.capacity = 10;
  test_signals[PROTON_SIGNALS__TEST__UINT64_LIST].arg.size = 0;

  test_signals[PROTON_SIGNALS__TEST__BOOL_LIST].signal.which_signal = proton_Signal_list_bool_value_tag;
  test_signals[PROTON_SIGNALS__TEST__BOOL_LIST].signal.signal.list_bool_value.bools = &test_signals[PROTON_SIGNALS__TEST__BOOL_LIST].arg;
  test_signals[PROTON_SIGNALS__TEST__BOOL_LIST].arg.data = test_struct.bool_list;
  test_signals[PROTON_SIGNALS__TEST__BOOL_LIST].arg.capacity = 10;
  test_signals[PROTON_SIGNALS__TEST__BOOL_LIST].arg.size = 0;

  test_signals[PROTON_SIGNALS__TEST__STRING_LIST].signal.which_signal = proton_Signal_list_string_value_tag;
  test_signals[PROTON_SIGNALS__TEST__STRING_LIST].signal.signal.list_string_value.strings = &test_signals[PROTON_SIGNALS__TEST__STRING_LIST].arg;
  test_signals[PROTON_SIGNALS__TEST__STRING_LIST].arg.data = test_struct.string_list.list;
  test_signals[PROTON_SIGNALS__TEST__STRING_LIST].arg.capacity = 10;
  test_signals[PROTON_SIGNALS__TEST__STRING_LIST].arg.size = 0;
  for (int i = 0; i < 10; i += 1)
  {
    test_struct.string_list.list[i] = test_struct.string_list.strings[i];
  }

  PROTON_InitBundle(&test_bundle, 4660, test_signals, PROTON_SIGNALS__TEST_COUNT);
}

void PROTON_MESSAGE_init()
{
  PROTON_BUNDLE_init_test();
}

