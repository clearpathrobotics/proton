#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <proton.h>

#define PROTON_MAX_MESSAGE_SIZE 1024

typedef enum ProtonMessage {
  PROTON_MESSAGE = 0x200,
} proton_message_e;

typedef enum ProtonSignals {
  PROTON_SIGNAL_BOOL_TEST,
  PROTON_SIGNAL_DOUBLE_TEST,
  PROTON_SIGNAL_FLOAT_TEST,
  PROTON_SIGNAL_INT32_TEST,
  PROTON_SIGNAL_INT64_TEST,
  PROTON_SIGNAL_UINT32_TEST,
  PROTON_SIGNAL_UINT64_TEST,
  PROTON_SIGNAL_STRING_TEST,
  PROTON_SIGNAL_BYTES_TEST,
  PROTON_SIGNAL_LIST_DOUBLE_TEST,
  PROTON_SIGNAL_LIST_FLOAT_TEST,
  PROTON_SIGNAL_LIST_INT32_TEST,
  PROTON_SIGNAL_LIST_INT64_TEST,
  PROTON_SIGNAL_LIST_UINT32_TEST,
  PROTON_SIGNAL_LIST_UINT64_TEST,
  PROTON_SIGNAL_LIST_BOOLS_TEST,
  PROTON_SIGNAL_LIST_STRING_TEST,
  PROTON_SIGNAL_COUNT
} proton_signal_e;

uint8_t read_buf_[PROTON_MAX_MESSAGE_SIZE];

/**** LISTS ****/
typedef struct TestProton
{
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
} test_proton_bufs;

test_proton_bufs bufs;

proton_signal_schema_t signal_schema[PROTON_SIGNAL_COUNT] = {
  {PROTON_SIGNAL_TYPE_BOOL_VALUE, proton_list_arg_init_default},
  {PROTON_SIGNAL_TYPE_DOUBLE_VALUE, proton_list_arg_init_default},
  {PROTON_SIGNAL_TYPE_FLOAT_VALUE, proton_list_arg_init_default},
  {PROTON_SIGNAL_TYPE_INT32_VALUE, proton_list_arg_init_default},
  {PROTON_SIGNAL_TYPE_INT64_VALUE, proton_list_arg_init_default},
  {PROTON_SIGNAL_TYPE_UINT32_VALUE, proton_list_arg_init_default},
  {PROTON_SIGNAL_TYPE_UINT64_VALUE, proton_list_arg_init_default},
  {PROTON_SIGNAL_TYPE_STRING_VALUE, {bufs.string_message, 64, 0}},
  {PROTON_SIGNAL_TYPE_BYTES_VALUE, {bufs.bytes_message, 64, 0}},
  {PROTON_SIGNAL_TYPE_LIST_DOUBLE_VALUE, {bufs.double_list, 10, 0}},
  {PROTON_SIGNAL_TYPE_LIST_FLOAT_VALUE, {bufs.float_list, 10, 0}},
  {PROTON_SIGNAL_TYPE_LIST_INT32_VALUE, {bufs.int32_list, 10, 0}},
  {PROTON_SIGNAL_TYPE_LIST_INT64_VALUE, {bufs.int64_list, 10, 0}},
  {PROTON_SIGNAL_TYPE_LIST_UINT32_VALUE, {bufs.uint32_list, 10, 0}},
  {PROTON_SIGNAL_TYPE_LIST_UINT64_VALUE, {bufs.uint64_list, 10, 0}},
  {PROTON_SIGNAL_TYPE_LIST_BOOL_VALUE, {bufs.bool_list, 10, 0}},
  {PROTON_SIGNAL_TYPE_LIST_STRING_VALUE, {bufs.string_list.list, 10, 0}},
};

proton_Signal signals[PROTON_SIGNAL_COUNT];
proton_bundle_t proton;

int sockfd;


void print_signal(proton_Signal signal);
void print_proton(proton_Bundle proton);


int socket_init()
{
  struct sockaddr_in servaddr;
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  servaddr.sin_port = htons(11417);

  if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
  {
    printf("bind error\r\n");
    return 1;
  }

  printf("Socket bound\r\n");

  return 0;
}

void TASKS_PROTON_Init()
{
  for (int i = 0; i < 10; i++)
  {
    bufs.string_list.list[i] = bufs.string_list.strings[i];
  }
  PROTON_InitProton(&proton, PROTON_MESSAGE, signals, signal_schema, PROTON_SIGNAL_COUNT);
}

void print_proton(proton_Bundle proton)
{
  proton_list_arg_t * args = (proton_list_arg_t *)proton.signals;
  printf("Proton message { \r\n");
  printf("\tID: 0x%x\r\n", proton.id);
  printf("\tSignals { \r\n");
  for (int i = 0; i < args->capacity; i++)
  {
    print_signal(((proton_Signal *)args->values)[i]);
  }
  printf("\t}\r\n}\r\n");
}

void print_signal(proton_Signal signal)
{
  pb_size_t which = signal.which_signal;

  proton_list_arg_t arg = proton_list_arg_init_default;

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
        arg = *(proton_list_arg_t *)signal.signal.string_value;
        printf("\t\tstring_value: %s\r\n", (char *)arg.values);
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
        arg = *(proton_list_arg_t *)signal.signal.bytes_value;
        printf("\t\tbytes_value: [");
        for (int i = 0; i < arg.size; i++)
        {
          printf("0x%x", ((uint8_t *)arg.values)[i]);
          if (i != arg.size - 1)
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
        arg = *(proton_list_arg_t *)signal.signal.list_double_value.doubles;
        printf("\t\tlist_double_value: {\r\n");
        for (int i = 0; i < arg.capacity; i++)
        {
          printf("\t\t\t%lf\r\n", ((double *)arg.values)[i]);
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
        arg = *(proton_list_arg_t *)signal.signal.list_float_value.floats;
        printf("\t\tlist_float_value: {\r\n");
        for (int i = 0; i < arg.capacity; i++)
        {
          printf("\t\t\t%f\r\n", ((float *)arg.values)[i]);
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
        arg = *(proton_list_arg_t *)signal.signal.list_int32_value.int32s;
        printf("\t\tlist_int32_value: {\r\n");
        for (int i = 0; i < arg.capacity; i++)
        {
          printf("\t\t\t%d\r\n", ((int32_t *)arg.values)[i]);
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
        arg = *(proton_list_arg_t *)signal.signal.list_int64_value.int64s;
        printf("\t\tlist_int64_value: {\r\n");
        for (int i = 0; i < arg.capacity; i++)
        {
          printf("\t\t\t%ld\r\n", ((int64_t *)arg.values)[i]);
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
        arg = *(proton_list_arg_t *)signal.signal.list_uint32_value.uint32s;
        printf("\t\tlist_uint32_value: {\r\n");
        for (int i = 0; i < arg.capacity; i++)
        {
          printf("\t\t\t%u\r\n", ((uint32_t *)arg.values)[i]);
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
        arg = *(proton_list_arg_t *)signal.signal.list_uint64_value.uint64s;
        printf("\t\tlist_uint64_value: {\r\n");
        for (int i = 0; i < arg.capacity; i++)
        {
          printf("\t\t\t%lu\r\n", ((uint64_t *)arg.values)[i]);
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
        arg = *(proton_list_arg_t *)signal.signal.list_bool_value.bools;
        printf("\t\tlist_bool_value: {\r\n");
        for (int i = 0; i < arg.capacity; i++)
        {
          printf("\t\t\t%u\r\n", ((bool *)arg.values)[i]);
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
        arg = *(proton_list_arg_t *)signal.signal.list_string_value.strings;
        printf("\t\tlist_string_value: {\r\n");
        for (int i = 0; i < arg.capacity; i++)
        {
          printf("\t\t\t%s\r\n", ((char **)arg.values)[i]);
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


int main()
{
  printf("~~~~~~~ Proton publisher ~~~~~~~\r\n");

  if (socket_init() != 0)
  {
    return -1;
  }

  TASKS_PROTON_Init();

  // printf("Proton %p\r\n", &proton.proton);
  // printf("Signal 1 %p\r\n", &signals[0]);
  // printf("Signal 2 %p\r\n", &signals[1]);
  // printf("List 1 %p\r\n", &float_list_signal);
  // printf("List 2 %p\r\n", &float_list_signal2);

  printf("INIT\r\n");

  while(1)
  {
    printf("\r\n~~~~~~~~~~~~~~~~~~~~~~~~~\r\n\r\n");
    int s = recv(sockfd, read_buf_, sizeof(read_buf_), 0);
    printf("Received %d bytes\r\n", s);
    int left = PROTON_Decode(&proton, read_buf_, s);
    if (left == -1)
    {
      printf("Decode failed\r\n");
    }
    else
    {
      printf("Decoded %d bytes\r\n", s - left);
      print_proton(proton.bundle);
    }
  }

  return 0;
}

