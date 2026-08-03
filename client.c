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
	State* s = all_states + i;
	struct 
	{
	  enum DB_Action type;
	  KEYVAL;
	} all_command [] =
	{
	  {
	    .type = SET,
	    .key = BUFFER("Hani"),
	    .value = BUFFER("Sayegh"),
	  },
	  // {
	  //   .type = DELETE,
	  //   .data.key = BUFFER("Hani"),
	  // },
	  // {
	  //   .type = SET,
	  //   .data.key = BUFFER("Sami"),
	  //   .data.value = BUFFER("Sayegh"),
	  // },
	};

	for(int i = 0; i < N(all_command); ++i)
	{
	  Message* msg = &s->msg;
	  *msg = (Message){};
	  add_byte(msg, &all_command[i].type, 4);
	  add_byte(msg, &all_command[i].type, 4);
	  add_Buffer(msg, all_command[i].key);
	  add_Buffer(msg, all_command[i].value);
	  s->msg.start[0] = s->msg.n_byte - 4;
	  printf("%s ", all_DB_Action[all_command[i].type]);
	  printf("%.*s ", all_command[i].key.n, all_command[i].key.start);
	  printf("%.*s ", all_command[i].value.n, all_command[i].value.start);
	  printf("\n");
	  set_state_writing(s);
	  do_partial_io(s);
	  if(try_to_transition(s) == MsgLen)
	  {
	    do_partial_io(s);
	    if(try_to_transition(s) == Msg)
	    {
	      do_partial_io(s);
	      Buffer response = {};
	      response.n = s->msg.n_byte_processed;
	      response.start = s->msg.start;
	      printf("%.*s\n", response.n, response.start);
	    }
	  }
	}
      }
    }
  }

  sleep(4);
  for (int i = 0; i < N_CLIENT; ++i)
  {
    close(all_states[i].fd);
  }
  return 0;
}
