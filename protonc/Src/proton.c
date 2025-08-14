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

#include "proton.h"
#include <stdio.h>

void PROTON_InitBundle(proton_bundle_t * bundle, uint32_t id, proton_signal_t * signals, uint32_t signal_count)
{
  if (signals && bundle)
  {
    bundle->arg.data = signals;
    bundle->arg.capacity = signal_count;
    bundle->arg.size = 0;
    bundle->bundle.id = id;
    bundle->bundle.signals = &bundle->arg;
  }
  else
  {
    // Error
  }
}

int PROTON_Encode(proton_bundle_t * bundle, uint8_t * buffer, size_t buffer_length)
{
  proton_signal_t * signal;

  // Copy non-list field values from struct to signal
  for (uint8_t i = 0; i < bundle->arg.capacity; i++)
  {
    signal = &((proton_signal_t *)bundle->arg.data)[i];
    switch(signal->signal.which_signal)
    {
      case proton_Signal_double_value_tag:
      {
        signal->signal.signal.double_value = *((double *)signal->arg.data);
        break;
      }

      case proton_Signal_float_value_tag:
      {
        signal->signal.signal.float_value = *((float *)signal->arg.data);
        break;
      }

      case proton_Signal_int32_value_tag:
      {
        signal->signal.signal.int32_value = *((int32_t *)signal->arg.data);
        break;
      }

      case proton_Signal_int64_value_tag:
      {
        signal->signal.signal.int64_value = *((int64_t *)signal->arg.data);
        break;
      }

      case proton_Signal_uint32_value_tag:
      {
        signal->signal.signal.uint32_value = *((uint32_t *)signal->arg.data);
        break;
      }

      case proton_Signal_uint64_value_tag:
      {
        signal->signal.signal.uint64_value = *((uint64_t *)signal->arg.data);
        break;
      }

      case proton_Signal_bool_value_tag:
      {
        signal->signal.signal.bool_value = *((bool *)signal->arg.data);
        break;
      }

      default:
        break;
    }
  }

  pb_ostream_t stream = pb_ostream_from_buffer((pb_byte_t *)buffer, buffer_length);
  bool status = pb_encode(&stream, proton_Bundle_fields, &bundle->bundle);
  size_t bytes_written = stream.bytes_written;

  if (status)
  {
    return bytes_written;
  }
  else
  {
    printf("Error: %s\r\n", stream.errmsg);
    return -1;
  }
}

int PROTON_Decode(proton_bundle_t * bundle, const uint8_t * buffer, size_t buffer_length)
{
  pb_istream_t stream = pb_istream_from_buffer((const pb_byte_t *)buffer, buffer_length);
  bool status = pb_decode(&stream, proton_Bundle_fields, &bundle->bundle);
  size_t bytes_left = stream.bytes_left;

  if (status)
  {
    proton_signal_t * signal;

    for (uint8_t i = 0; i < bundle->arg.capacity; i++)
    {
      signal = &((proton_signal_t *)bundle->arg.data)[i];
      switch(signal->signal.which_signal)
      {
        case proton_Signal_double_value_tag:
        {
          *((double *)signal->arg.data) = signal->signal.signal.double_value;
          break;
        }

        case proton_Signal_float_value_tag:
        {
          *((float *)signal->arg.data) = signal->signal.signal.float_value;
          break;
        }

        case proton_Signal_int32_value_tag:
        {
          *((int32_t *)signal->arg.data) = signal->signal.signal.int32_value;
          break;
        }

        case proton_Signal_int64_value_tag:
        {
          *((int64_t *)signal->arg.data) = signal->signal.signal.int64_value;
          break;
        }

        case proton_Signal_uint32_value_tag:
        {
          *((uint32_t *)signal->arg.data) = signal->signal.signal.uint32_value;
          break;
        }

        case proton_Signal_uint64_value_tag:
        {
          *((uint64_t *)signal->arg.data) = signal->signal.signal.uint64_value;
          break;
        }

        case proton_Signal_bool_value_tag:
        {
          *((bool *)signal->arg.data) = signal->signal.signal.bool_value;
          break;
        }

        default:
          break;
      }
    }

    return bytes_left;
  }
  else
  {
    printf("Error: %s\r\n", stream.errmsg);
    return -1;
  }
}

void print_bundle(proton_Bundle bundle)
{
  proton_list_t * args = (proton_list_t *)bundle.signals;
  printf("Proton message { \r\n");
  printf("\tID: 0x%x\r\n", bundle.id);
  printf("\tSignals { \r\n");
  for (int i = 0; i < args->capacity; i++)
  {
    print_signal(((proton_signal_t *)args->data)[i].signal);
  }
  printf("\t}\r\n}\r\n");
}

void print_signal(proton_Signal signal)
{
  pb_size_t which = signal.which_signal;

  proton_list_t arg = proton_list_arg_init_default;

  switch(which)
  {
    case proton_Signal_bool_value_tag:
    {
      printf("\t\tbool_value: %d\r\n", signal.signal.bool_value);
      break;
    }

    case proton_Signal_double_value_tag:
    {
      printf("\t\tdouble_value: %lf\r\n", signal.signal.double_value);
      break;
    }

    case proton_Signal_float_value_tag:
    {
      printf("\t\tfloat_value: %f\r\n", signal.signal.float_value);
      break;
    }

    case proton_Signal_int32_value_tag:
    {
      printf("\t\tint32_value: %d\r\n", signal.signal.int32_value);
      break;
    }

    case proton_Signal_int64_value_tag:
    {
      printf("\t\tint64_value: %ld\r\n", signal.signal.int64_value);
      break;
    }

    case proton_Signal_uint32_value_tag:
    {
      printf("\t\tuint32_value: %u\r\n", signal.signal.uint32_value);
      break;
    }

    case proton_Signal_uint64_value_tag:
    {
      printf("\t\tuint64_value: %lu\r\n", signal.signal.uint64_value);
      break;
    }

    case proton_Signal_string_value_tag:
    {
      if (signal.signal.string_value)
      {
        arg = *(proton_list_t *)signal.signal.string_value;
        printf("\t\tstring_value: %s\r\n", (char *)arg.data);
      }
      else
      {
        printf("\t\tNULL string\r\n");
      }
      break;
    }

    case proton_Signal_bytes_value_tag:
    {
      if (signal.signal.bytes_value)
      {
        arg = *(proton_list_t *)signal.signal.bytes_value;
        printf("\t\tbytes_value: [");
        for (int i = 0; i < arg.capacity; i++)
        {
          printf("0x%x", ((uint8_t *)arg.data)[i]);
          if (i != arg.capacity - 1)
          {
            printf(", ");
          }
        }
        printf("]\r\n");
      }
      else
      {
        printf("\t\tNULL bytes\r\n");
      }
      break;
    }

    case proton_Signal_list_double_value_tag:
    {
      if (signal.signal.list_double_value.doubles)
      {
        arg = *(proton_list_t *)signal.signal.list_double_value.doubles;
        printf("\t\tlist_double_value: {\r\n");
        for (int i = 0; i < arg.capacity; i++)
        {
          printf("\t\t\t%lf\r\n", ((double *)arg.data)[i]);
        }
        printf("\t\t}\r\n");
      }
      else
      {
        printf("\t\tNULL double array \r\n");
      }
      break;
    }

    case proton_Signal_list_float_value_tag:
    {
      if (signal.signal.list_float_value.floats)
      {
        arg = *(proton_list_t *)signal.signal.list_float_value.floats;
        printf("\t\tlist_float_value: {\r\n");
        for (int i = 0; i < arg.capacity; i++)
        {
          printf("\t\t\t%f\r\n", ((float *)arg.data)[i]);
        }
        printf("\t\t}\r\n");
      }
      else
      {
        printf("\t\tNULL float array \r\n");
      }
      break;
    }

    case proton_Signal_list_int32_value_tag:
    {
      if (signal.signal.list_int32_value.int32s)
      {
        arg = *(proton_list_t *)signal.signal.list_int32_value.int32s;
        printf("\t\tlist_int32_value: {\r\n");
        for (int i = 0; i < arg.capacity; i++)
        {
          printf("\t\t\t%d\r\n", ((int32_t *)arg.data)[i]);
        }
        printf("\t\t}\r\n");
      }
      else
      {
        printf("\t\tNULL int32 array \r\n");
      }
      break;
    }

    case proton_Signal_list_int64_value_tag:
    {
      if (signal.signal.list_int64_value.int64s)
      {
        arg = *(proton_list_t *)signal.signal.list_int64_value.int64s;
        printf("\t\tlist_int64_value: {\r\n");
        for (int i = 0; i < arg.capacity; i++)
        {
          printf("\t\t\t%ld\r\n", ((int64_t *)arg.data)[i]);
        }
        printf("\t\t}\r\n");
      }
      else
      {
        printf("\t\tNULL int64 array \r\n");
      }
      break;
    }

    case proton_Signal_list_uint32_value_tag:
    {
      if (signal.signal.list_uint32_value.uint32s)
      {
        arg = *(proton_list_t *)signal.signal.list_uint32_value.uint32s;
        printf("\t\tlist_uint32_value: {\r\n");
        for (int i = 0; i < arg.capacity; i++)
        {
          printf("\t\t\t%u\r\n", ((uint32_t *)arg.data)[i]);
        }
        printf("\t\t}\r\n");
      }
      else
      {
        printf("\t\tNULL uint32 array \r\n");
      }
      break;
    }

    case proton_Signal_list_uint64_value_tag:
    {
      if (signal.signal.list_uint64_value.uint64s)
      {
        arg = *(proton_list_t *)signal.signal.list_uint64_value.uint64s;
        printf("\t\tlist_uint64_value: {\r\n");
        for (int i = 0; i < arg.capacity; i++)
        {
          printf("\t\t\t%lu\r\n", ((uint64_t *)arg.data)[i]);
        }
        printf("\t\t}\r\n");
      }
      else
      {
        printf("\t\tNULL uint64 array \r\n");
      }
      break;
    }

    case proton_Signal_list_bool_value_tag:
    {
      if (signal.signal.list_bool_value.bools)
      {
        arg = *(proton_list_t *)signal.signal.list_bool_value.bools;
        printf("\t\tlist_bool_value: {\r\n");
        for (int i = 0; i < arg.capacity; i++)
        {
          printf("\t\t\t%u\r\n", ((bool *)arg.data)[i]);
        }
        printf("\t\t}\r\n");
      }
      else
      {
        printf("\t\tNULL bool array \r\n");
      }
      break;
    }

    case proton_Signal_list_string_value_tag:
    {
      if (signal.signal.list_string_value.strings)
      {
        arg = *(proton_list_t *)signal.signal.list_string_value.strings;
        printf("\t\tlist_string_value: {\r\n");
        for (int i = 0; i < arg.capacity; i++)
        {
          printf("\t\t\t%s\r\n", ((char **)arg.data)[i]);
        }
        printf("\t\t}\r\n");
      }
      else
      {
        printf("\t\tNULL string\r\n");
      }
      break;
    }
  }
}