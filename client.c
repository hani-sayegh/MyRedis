#include "stdio.h"
#include "string.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "unistd.h"
#include "errno.h"

#include "common.c"

#define SOCK_COUNT 10

int main()
{
  int all_sockets[SOCK_COUNT] = {};

  int gd = 1;
  for (int i = 0; i < SOCK_COUNT; ++i)
  {
    all_sockets[i] = socket(AF_INET, SOCK_STREAM, 0);
    if (all_sockets[i] == -1)
    {
      gd = 0;
      perror(__FILE__);
    }
  }

  if (gd)
  {
    struct sockaddr_in add = {};
    add.sin_family         = AF_INET;
    add.sin_port           = htons(1234);
    add.sin_addr.s_addr    = htonl(INADDR_LOOPBACK);

    for (int i = 0; i < SOCK_COUNT; ++i)
    {
      gd = connect(all_sockets[i], (const struct sockaddr *)&add,
                   sizeof(struct sockaddr_in));

      if (gd == -1)
      {
        perror(__FILE__);
      }
    }

    // else
    // {
    //   int repeat = 1;
    //   while (repeat--)
    //   {
    //
    //     char hi[] = "hello";
    //     send_payload(fd, hi, strlen(hi));
    //     process_payload(fd);
    //   }
    // }
  }

  for (int i = 0; i < SOCK_COUNT; ++i)
  {
    close(all_sockets[i]);
  }
  return 0;
}
