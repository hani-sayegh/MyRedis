#include "stdio.h"
#include "string.h"
#include "errno.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "unistd.h"

#include "common.c"

int main()
{
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  // this is needed for most server applications
  int val = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

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
        bind(fd, (const struct sockaddr *)&add, sizeof(struct sockaddr_in));
    if (err == -1)
    {
      perror(__FILE__);
    }
    else
    {
      int err = listen(fd, SOMAXCONN);
      if (err == -1)
      {
        perror(__FILE__);
      }
      else
      {
        printf("Server waiting for connections\n");
        while (1)
        {
          struct sockaddr_in client_info = {};
          socklen_t addrlen              = sizeof(client_info);

          int fd_client = accept(fd, (struct sockaddr *)&client_info, &addrlen);
          printf("[%d] has connected\n", fd_client);
          if (fd_client == -1)
          {
            perror(__FILE__);
          }
          else
          {
            process_payload(fd_client);
            char message_to_client[] = "The server sent you message!\n";
            send_payload(fd_client, message_to_client,
                         strlen(message_to_client));
          }
          close(fd_client);
          printf("[%d] has disconnected\n", fd_client);
        }
      }
    }
  }
  printf("Exiting\n");
  close(fd);
  return 0;
}
