#include "stdio.h"
#include "string.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "unistd.h"
#include "errno.h"

#include "common.c"

#define SOCK_COUNT 1000

int main()
{
  int all_sockets[SOCK_COUNT] = {};

  int i = 0;
  for (; i < SOCK_COUNT; ++i)
  {
    all_sockets[i] = socket(AF_INET, SOCK_STREAM , 0);
    if (all_sockets[i] == -1)
    {
      perror(__FILE__);
      break;
    }
  }

  if (i == SOCK_COUNT)
  {
    struct sockaddr_in add = {};
    add.sin_family         = AF_INET;
    add.sin_port           = htons(1234);
    add.sin_addr.s_addr    = htonl(INADDR_LOOPBACK);

    for (i = 0; i < SOCK_COUNT; ++i)
    {
      int err = connect(all_sockets[i], (const struct sockaddr *)&add,
                        sizeof(struct sockaddr_in));

      if (err == -1)
      {
        perror(__FILE__);
        break;
      }
    }

    if (i == SOCK_COUNT)
    {
      for (i = 0; i < SOCK_COUNT; ++i)
      {
        char hi[] = "hello";
        send_payload(all_sockets[i], hi, strlen(hi));
        process_payload(all_sockets[i]);
      }
    }
  }

  for (int i = 0; i < SOCK_COUNT; ++i)
  {
    close(all_sockets[i]);
  }
  return 0;
}
