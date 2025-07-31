/**
 * Software License Agreement (proprietary)
 *
 * @copyright Copyright (c) 2025 Clearpath Robotics, Inc., All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, is not permitted without the
 * express permission of Clearpath Robotics.
 *
 * @author Roni Kreinin (rkreinin@clearpathrobotics.com)
 */

#ifndef INC_PROTON_PROTON_H_
#define INC_PROTON_PROTON_H_

#include "proton.pb.h"
#include "pb.h"
#include "pb_decode.h"
#include "pb_encode.h"


typedef enum {
  PROTON_SIGNAL_TYPE_DOUBLE_VALUE,
  PROTON_SIGNAL_TYPE_FLOAT_VALUE,
  PROTON_SIGNAL_TYPE_INT32_VALUE,
  PROTON_SIGNAL_TYPE_INT64_VALUE,
  PROTON_SIGNAL_TYPE_UINT32_VALUE,
  PROTON_SIGNAL_TYPE_UINT64_VALUE,
  PROTON_SIGNAL_TYPE_BOOL_VALUE,
  PROTON_SIGNAL_TYPE_STRING_VALUE,
  PROTON_SIGNAL_TYPE_BYTES_VALUE,
  PROTON_SIGNAL_TYPE_LIST_DOUBLE_VALUE,
  PROTON_SIGNAL_TYPE_LIST_FLOAT_VALUE,
  PROTON_SIGNAL_TYPE_LIST_INT32_VALUE,
  PROTON_SIGNAL_TYPE_LIST_INT64_VALUE,
  PROTON_SIGNAL_TYPE_LIST_UINT32_VALUE,
  PROTON_SIGNAL_TYPE_LIST_UINT64_VALUE,
  PROTON_SIGNAL_TYPE_LIST_BOOL_VALUE,
  PROTON_SIGNAL_TYPE_LIST_STRING_VALUE,
} proton_signal_type_e;

typedef struct ListArg {
  void * values;
  size_t capacity;
  size_t size;
} proton_list_arg_t;

#define proton_list_arg_init_default {NULL, 0, 0}

typedef struct {
  proton_signal_type_e type;
  proton_list_arg_t arg;
} proton_signal_schema_t;

typedef struct Proton {
  proton_Proton proton;
  proton_list_arg_t arg;
} proton_t;


#define INIT_PROTON(id, schema) (proton_t){ \
  .proton.id = id,                           \
  .proton.signals.                                      \
}

void PROTON_InitProton(proton_t * proton, uint32_t id, proton_Signal * signals, proton_signal_schema_t * schema, uint32_t signal_count);
void PROTON_InitSignal(proton_Signal * signal, proton_signal_schema_t * schema);

bool PROTON_CopyStringToSignal(proton_Signal * signal, const char * string);
bool PROTON_CopyStringToListString(proton_Signal * signal, const char * string, size_t index);

int PROTON_Encode(proton_Proton * msg, uint8_t * buffer, size_t buffer_length);
int PROTON_Decode(proton_Proton * msg, const uint8_t * buffer, const size_t buffer_length);

// Struct: {{(proton_Signal){pb_size_t, (union){signal}}, {(proton_list_arg_t{void *, size_t}}
#define proton_bool_signal_init_default {{proton_Signal_bool_value_tag, {false}}, {NULL, 0}}
#define proton_list_float_value_signal_init_default {{{proton_Signal_list_float_value_tag, {{NULL}, NULL}}}, {NULL, 0}}

#define proton_signal_init_default(type) \
  ((type) == PROTON_SIGNAL_TYPE_BOOL_VALUE ? (signal_t)proton_bool_signal_init_default : \
   (type) == PROTON_SIGNAL_TYPE_LIST_FLOAT_VALUE ? (signal_t)proton_list_float_value_signal_init_default \
  : (signal_t)proton_bool_signal_init_default)


#endif  // INC_PROTON_PROTON_H_
