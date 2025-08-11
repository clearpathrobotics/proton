#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "proton__sample.h"

#define PROTON_MAX_MESSAGE_SIZE 1024

uint8_t write_buf_[PROTON_MAX_MESSAGE_SIZE];

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

int fill_proton_message()
{
  strcpy(test.string_message, "STRING_MESSAGE");

  test.bytes_message[10] = 0xFF;

  test.double_list[0] = 0.11234123412341234;
  test.double_list[1] = 0.21234123412341234;
  test.double_list[2] = 0.311234123412341234;
  test.double_list[3] = 0.441234123412341234;
  test.double_list[4] = 0.551234123412341234;
  test.double_list[5] = 0.661234123412341234;
  test.double_list[6] = 0.771234123412341234;
  test.double_list[7] = 0.991234123412341234;
  test.double_list[8] = 0.881234123412341234;
  test.double_list[9] = 0.123125131234123412341234;

  test.float_list[0] = 0.1f;
  test.float_list[1] = 0.2f;
  test.float_list[2] = 0.31f;
  test.float_list[3] = 0.44f;
  test.float_list[4] = 0.55f;
  test.float_list[5] = 0.66f;
  test.float_list[6] = 0.77f;
  test.float_list[7] = 0.99f;
  test.float_list[8] = 0.88f;
  test.float_list[9] = 0.12312513f;

  test.int32_list[0] = -4;
  test.int32_list[1] = -3;
  test.int32_list[2] = -2;
  test.int32_list[3] = -1;
  test.int32_list[4] = 0;
  test.int32_list[5] = 1;
  test.int32_list[6] = 2;
  test.int32_list[7] = 3;
  test.int32_list[8] = 4;
  test.int32_list[9] = 5;

  test.int64_list[0] = INT64_MAX;
  test.int64_list[1] = INT64_MAX;
  test.int64_list[2] = INT64_MAX;
  test.int64_list[3] = INT64_MAX;
  test.int64_list[4] = INT64_MAX;
  test.int64_list[5] = 12;
  test.int64_list[6] = INT64_MIN;
  test.int64_list[7] = INT64_MIN;
  test.int64_list[8] = INT64_MIN;
  test.int64_list[9] = INT64_MIN;

  test.uint32_list[0] = UINT32_MAX;
  test.uint32_list[1] = UINT32_MAX;
  test.uint32_list[2] = UINT32_MAX;
  test.uint32_list[3] = UINT32_MAX;
  test.uint32_list[4] = 0;
  test.uint32_list[5] = UINT32_MAX;
  test.uint32_list[6] = UINT32_MAX;
  test.uint32_list[7] = UINT32_MAX;
  test.uint32_list[8] = UINT32_MAX;
  test.uint32_list[9] = UINT32_MAX;

  test.uint64_list[0] = UINT64_MAX;
  test.uint64_list[1] = UINT64_MAX;
  test.uint64_list[2] = UINT64_MAX;
  test.uint64_list[3] = UINT64_MAX;
  test.uint64_list[4] = UINT64_MAX;
  test.uint64_list[5] = 12;
  test.uint64_list[6] = UINT64_MAX;
  test.uint64_list[7] = UINT64_MAX;
  test.uint64_list[8] = UINT64_MAX;
  test.uint64_list[9] = UINT64_MAX;

  test.bool_list[0] = false;
  test.bool_list[1] = false;
  test.bool_list[2] = false;
  test.bool_list[3] = false;
  test.bool_list[4] = false;
  test.bool_list[5] = true;
  test.bool_list[6] = true;
  test.bool_list[7] = true;
  test.bool_list[8] = true;
  test.bool_list[9] = true;

  strcpy(test.string_list.strings[0], "TEST1");
  strcpy(test.string_list.strings[1], "TEST2");
  strcpy(test.string_list.strings[2], "TEST3");
  strcpy(test.string_list.strings[3], "TEST4");
  strcpy(test.string_list.strings[4], "TEST5");
  strcpy(test.string_list.strings[5], "TEST6");
  strcpy(test.string_list.strings[6], "TEST7");
  strcpy(test.string_list.strings[7], "TEST8");
  strcpy(test.string_list.strings[8], "TEST9");
  strcpy(test.string_list.strings[9], "TEST10");


  return PROTON_Encode(&test_bundle, write_buf_, sizeof(write_buf_));
}

int main()
{
  printf("~~~~~~~ Proton publisher ~~~~~~~\r\n");

  if (socket_init() != 0)
  {
    return -1;
  }

  PROTON_MESSAGE_init();

  printf("INIT\r\n");

  int s = fill_proton_message();


  printf("Encoded %d bytes\r\n", s);

  if (s > 0)
  {
    send(sockfd, write_buf_, s, 0);
  }

  return 0;
}

