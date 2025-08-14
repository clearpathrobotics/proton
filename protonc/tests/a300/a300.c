#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <stdlib.h>

#include "proton__a300.h"

#define PROTON_MAX_MESSAGE_SIZE 1024

uint8_t write_buf_[PROTON_MAX_MESSAGE_SIZE];

int sockfd;

int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res);

    return res;
}

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

int send_bundle(proton_bundle_t * bundle)
{
  int e = PROTON_Encode(bundle, write_buf_, sizeof(write_buf_));

  if (e > 0)
  {
    send(sockfd, write_buf_, e, 0);
  }

  return e;
}

int fill_log()
{
  strcpy(logger_struct.log, "TEST_LOG");
}

int update_power()
{
  for (uint8_t i = 0; i < 13; i++)
  {
    power_struct.measured_currents[i] = (float)rand();
    power_struct.measured_voltages[i] = (float)rand();
  }
  send_bundle(&power_bundle);
}

int update_temperature()
{
  for (uint8_t i = 0; i < 18; i++)
  {
    temperature_struct.temperatures[i] = (float)rand();
  }
  send_bundle(&temperature_bundle);
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

  uint64_t i = 0;

  while (1)
  {
    int e = fill_log();

    // 1 hz
    if (i % 1000 == 0)
    {
      send_bundle(&logger_bundle);
    }

    // 10hz
    if (i % 100 == 0)
    {
      update_power();
      update_temperature();
    }

    // 50hz
    if (i % 20 == 0)
    {

    }

    msleep(1);
    i++;
  }

  return 0;
}

