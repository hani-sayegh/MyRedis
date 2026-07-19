#include "stdio.h"
#include "string.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "unistd.h"
#include "errno.h"

#include "common.c"

#define N_CLIENT 1

int main()
{
  int all_sockets[N_CLIENT] = {};

  int i = 0;
  for (; i < N_CLIENT; ++i)
  {
    all_sockets[i] = socket(AF_INET, SOCK_STREAM , 0);
    if (all_sockets[i] == -1)
    {
      perror(__FILE__);
      break;
    }
  }

  if (i == N_CLIENT)
  {
    struct sockaddr_in add = {};
    add.sin_family         = AF_INET;
    add.sin_port           = htons(1234);
    add.sin_addr.s_addr    = htonl(INADDR_LOOPBACK);

    for (i = 0; i < N_CLIENT; ++i)
    {
      int err = connect(all_sockets[i], (const struct sockaddr *)&add,
	  sizeof(struct sockaddr_in));

      if (err == -1)
      {

	perror(__FILE__);
	if(errno == ECONNREFUSED)
	{
	  printf("Retrying.....\n");
	  sleep(1);
	  --i;
	}
	else
	{
	  break;
	}
      }
    }

    if (i == N_CLIENT)
    {
      for (i = 0; i < N_CLIENT; ++i)
      {
	State s = {};
	s.fd = all_sockets[i];
	char data [] = "hello my name is Hani";
	s.n_bytes = sizeof(s.n_bytes) + sizeof(data);
	s.data = calloc(s.n_bytes, 1);

	memcpy(s.data, &s.n_bytes, sizeof(s.n_bytes));
	memcpy(s.data + 4, data, sizeof(data));

	write_partial(&s);
	// process_payload(all_sockets[i]);
      }
    }
  }

  for (int i = 0; i < N_CLIENT; ++i)
  {
    close(all_sockets[i]);
  }
  return 0;
}
