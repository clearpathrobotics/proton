#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "proton.h"

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
  PROTON_SIGNAL_LIST_BOOL_TEST,
  PROTON_SIGNAL_LIST_STRING_TEST,
  PROTON_SIGNAL_COUNT
} proton_signal_e;

uint8_t write_buf_[PROTON_MAX_MESSAGE_SIZE];

/**** Lists ****/

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
  {PROTON_SIGNAL_TYPE_BYTES_VALUE, {bufs.bytes_message, 64, 64}},
  {PROTON_SIGNAL_TYPE_LIST_DOUBLE_VALUE, {bufs.double_list, 10}},
  {PROTON_SIGNAL_TYPE_LIST_FLOAT_VALUE, {bufs.float_list, 10}},
  {PROTON_SIGNAL_TYPE_LIST_INT32_VALUE, {bufs.int32_list, 10}},
  {PROTON_SIGNAL_TYPE_LIST_INT64_VALUE, {bufs.int64_list, 10}},
  {PROTON_SIGNAL_TYPE_LIST_UINT32_VALUE, {bufs.uint32_list, 10}},
  {PROTON_SIGNAL_TYPE_LIST_UINT64_VALUE, {bufs.uint64_list, 10}},
  {PROTON_SIGNAL_TYPE_LIST_BOOL_VALUE, {bufs.bool_list, 10}},
  {PROTON_SIGNAL_TYPE_LIST_STRING_VALUE, {bufs.string_list.list, 10, 64}},
};

proton_Signal signals[PROTON_SIGNAL_COUNT];
proton_bundle_t proton;

int sockfd;

int socket_init()
{
  struct sockaddr_in servaddr;
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  servaddr.sin_port = htons(11417);

  if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
  {
    printf("connect error\r\n");
    return 1;
  }

  printf("Socket connected\r\n");

  return 0;
}

void TASKS_PROTON_Init()
{
  for (int i = 0; i < 10; i++)
  {
    bufs.string_list.list[i] = bufs.string_list.strings[i];
  }
  PROTON_InitBundle(&proton, PROTON_MESSAGE, signals, signal_schema, PROTON_SIGNAL_COUNT);
}

int fill_proton_message()
{
  signals[PROTON_SIGNAL_BOOL_TEST].signal.bool_value = true;
  signals[PROTON_SIGNAL_DOUBLE_TEST].signal.double_value = 1.23456789;
  signals[PROTON_SIGNAL_FLOAT_TEST].signal.float_value = 1.2345f;
  signals[PROTON_SIGNAL_INT32_TEST].signal.int32_value = -1234;
  signals[PROTON_SIGNAL_INT64_TEST].signal.int64_value = -12345678912345;
  signals[PROTON_SIGNAL_UINT32_TEST].signal.uint32_value = 1234;
  signals[PROTON_SIGNAL_UINT64_TEST].signal.uint64_value = 12345678912345;

  PROTON_CopyStringToSignal(&signals[PROTON_SIGNAL_STRING_TEST], "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  bufs.bytes_message[10] = 0xFF;

  bufs.double_list[0] = 0.11234123412341234;
  bufs.double_list[1] = 0.21234123412341234;
  bufs.double_list[2] = 0.311234123412341234;
  bufs.double_list[3] = 0.441234123412341234;
  bufs.double_list[4] = 0.551234123412341234;
  bufs.double_list[5] = 0.661234123412341234;
  bufs.double_list[6] = 0.771234123412341234;
  bufs.double_list[7] = 0.991234123412341234;
  bufs.double_list[8] = 0.881234123412341234;
  bufs.double_list[9] = 0.123125131234123412341234;

  bufs.float_list[0] = 0.1f;
  bufs.float_list[1] = 0.2f;
  bufs.float_list[2] = 0.31f;
  bufs.float_list[3] = 0.44f;
  bufs.float_list[4] = 0.55f;
  bufs.float_list[5] = 0.66f;
  bufs.float_list[6] = 0.77f;
  bufs.float_list[7] = 0.99f;
  bufs.float_list[8] = 0.88f;
  bufs.float_list[9] = 0.12312513f;

  bufs.int32_list[0] = -4;
  bufs.int32_list[1] = -3;
  bufs.int32_list[2] = -2;
  bufs.int32_list[3] = -1;
  bufs.int32_list[4] = 0;
  bufs.int32_list[5] = 1;
  bufs.int32_list[6] = 2;
  bufs.int32_list[7] = 3;
  bufs.int32_list[8] = 4;
  bufs.int32_list[9] = 5;

  bufs.int64_list[0] = INT64_MAX;
  bufs.int64_list[1] = INT64_MAX;
  bufs.int64_list[2] = INT64_MAX;
  bufs.int64_list[3] = INT64_MAX;
  bufs.int64_list[4] = INT64_MAX;
  bufs.int64_list[5] = 12;
  bufs.int64_list[6] = INT64_MIN;
  bufs.int64_list[7] = INT64_MIN;
  bufs.int64_list[8] = INT64_MIN;
  bufs.int64_list[9] = INT64_MIN;

  bufs.uint32_list[0] = UINT32_MAX;
  bufs.uint32_list[1] = UINT32_MAX;
  bufs.uint32_list[2] = UINT32_MAX;
  bufs.uint32_list[3] = UINT32_MAX;
  bufs.uint32_list[4] = 0;
  bufs.uint32_list[5] = UINT32_MAX;
  bufs.uint32_list[6] = UINT32_MAX;
  bufs.uint32_list[7] = UINT32_MAX;
  bufs.uint32_list[8] = UINT32_MAX;
  bufs.uint32_list[9] = UINT32_MAX;

  bufs.uint64_list[0] = UINT64_MAX;
  bufs.uint64_list[1] = UINT64_MAX;
  bufs.uint64_list[2] = UINT64_MAX;
  bufs.uint64_list[3] = UINT64_MAX;
  bufs.uint64_list[4] = UINT64_MAX;
  bufs.uint64_list[5] = 12;
  bufs.uint64_list[6] = UINT64_MAX;
  bufs.uint64_list[7] = UINT64_MAX;
  bufs.uint64_list[8] = UINT64_MAX;
  bufs.uint64_list[9] = UINT64_MAX;

  bufs.bool_list[0] = false;
  bufs.bool_list[1] = false;
  bufs.bool_list[2] = false;
  bufs.bool_list[3] = false;
  bufs.bool_list[4] = false;
  bufs.bool_list[5] = true;
  bufs.bool_list[6] = true;
  bufs.bool_list[7] = true;
  bufs.bool_list[8] = true;
  bufs.bool_list[9] = true;

  PROTON_CopyStringToListString(&signals[PROTON_SIGNAL_LIST_STRING_TEST], "TEST1", 0);
  PROTON_CopyStringToListString(&signals[PROTON_SIGNAL_LIST_STRING_TEST], "TEST2", 1);
  PROTON_CopyStringToListString(&signals[PROTON_SIGNAL_LIST_STRING_TEST], "TEST3", 2);
  PROTON_CopyStringToListString(&signals[PROTON_SIGNAL_LIST_STRING_TEST], "TEST4", 3);
  PROTON_CopyStringToListString(&signals[PROTON_SIGNAL_LIST_STRING_TEST], "TEST5", 4);
  PROTON_CopyStringToListString(&signals[PROTON_SIGNAL_LIST_STRING_TEST], "TEST6", 5);
  PROTON_CopyStringToListString(&signals[PROTON_SIGNAL_LIST_STRING_TEST], "TEST7", 6);
  PROTON_CopyStringToListString(&signals[PROTON_SIGNAL_LIST_STRING_TEST], "TEST8", 7);
  PROTON_CopyStringToListString(&signals[PROTON_SIGNAL_LIST_STRING_TEST], "TEST9", 8);
  PROTON_CopyStringToListString(&signals[PROTON_SIGNAL_LIST_STRING_TEST], "TEST110", 9);

  return PROTON_Encode(&proton, write_buf_, sizeof(write_buf_));
}

int main()
{
  printf("~~~~~~~ Proton publisher ~~~~~~~\r\n");

  if (socket_init() != 0)
  {
    return -1;
  }

  TASKS_PROTON_Init();

  printf("INIT\r\n");

  int s = fill_proton_message();


  printf("Encoded %d bytes\r\n", s);

  if (s > 0)
  {
    send(sockfd, write_buf_, s, 0);
  }

  return 0;
}

