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
	int n_data = sizeof(data) - 1;
	s.n_bytes = sizeof(int) + n_data;
	s.data = malloc(s.n_bytes);

	memcpy(s.data, &n_data, sizeof(n_data));
	memcpy(s.data + 4, data, n_data);

	s.request = Send;
	do_partial_io(&s);
	s.request = MsgLen;
	do_partial_io(&s);
	s.request = Msg;
	do_partial_io(&s);

	printf("Awesomeeeee....: %s\n", s.data);
      }
    }
  }

  for (int i = 0; i < N_CLIENT; ++i)
  {
    close(all_sockets[i]);
  }
  return 0;
}
