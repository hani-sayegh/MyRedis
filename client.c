#include "stdio.h"
#include "string.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "unistd.h"
#include "errno.h"

int main()
{
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1)
  {
    printf("failed");
  }
  else
  {
    struct sockaddr_in add = {};
    add.sin_family         = AF_INET;
    add.sin_port           = htons(1234);
    add.sin_addr.s_addr    = htonl(INADDR_LOOPBACK);

    int err =
        connect(fd, (const struct sockaddr *)&add, sizeof(struct sockaddr_in));
    if (err == -1)
    {
      perror(__FILE__);
    }
    else
    {
      int repeat = 2;
      while (repeat--)
      {
        char msg[] = "Message from client";
        int err    = write(fd, msg, strlen(msg));
        if (err == -1)
        {
          perror(__FILE__);
        }
        else
        {
          char rbuf[1000] = {};
          int err         = read(fd, rbuf, sizeof(rbuf) - 1);
	  printf("[%d] bytes read\n", err);
          if (err == -1)
          {
            perror(__FILE__);
          }
          else
          {
            printf("SERVER: %s", rbuf);
          }
        }
      }
    }
  }
  close(fd);
  return 0;
}
