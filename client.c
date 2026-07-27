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
  State all_states[N_CLIENT] = {};

  int i = 0;
  for (; i < N_CLIENT; ++i)
  {
    all_states[i].fd = socket(AF_INET, SOCK_STREAM , 0);
    if (all_states[i].fd == -1)
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
      int err = connect(all_states[i].fd, (const struct sockaddr *)&add,
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
	State * s = all_states + i;
	struct 
	{
	  enum DB_Action type;
	  KeyVal data;
	} all_command [] =
	{
	  {
	    .type = SET,
	    .data.key = BUFFER("Hani"),
	    .data.value = BUFFER("Sayegh"),
	  },
	  {
	    .type = SET,
	    .data.key = BUFFER("Sami"),
	    .data.value = BUFFER("Sayegh"),
	  }
	};

	for(int i = 0; i < 1; ++i)
	{
	  *s = (State){};
	  add_byte(s, &all_command[i].type, 4);
	  add_byte(s, &all_command[i].type, 4);
	  add_Buffer(s, all_command[i].data.key);
	  add_Buffer(s, all_command[i].data.value);
	  s->start[0] = s->n_byte - 4;
	  set_state_writing(s);
	  do_partial_io(s);
	}
      }
    }
  }

  for (int i = 0; i < N_CLIENT; ++i)
  {
    close(all_states[i].fd);
  }
  return 0;
}
