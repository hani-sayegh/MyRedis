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
	  //   .key = BUFFER("Hani"),
	  // },
	  {
	    .type = SET,
	    .key = BUFFER("Sami"),
	    .value = BUFFER("Sayegh"),
	  },
	  // {
	  //   .type = GET,
	  //   .key = BUFFER("Sami"),
	  // },
	  {
	    .type = GET_ALL_KEY,
	  },
	};

	for(int i = 0; i < N(all_command); ++i)
	{
	  Message* msg = &s->msg;
	  *msg = (Message){};
	  add_n_byte(msg, &all_command[i].type, 4);
	  add_Buffer(msg, all_command[i].key);
	  add_Buffer(msg, all_command[i].value);
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
	      enum Type type;
	      parse_Type(&type, msg);
	      switch(type)
	      {
		case TYPE_STRING:
		  int n_msg;
		  memcpy(&n_msg, s->msg.start + sizeof(Type), sizeof(int));
		  printf("%.*s\n", n_msg, s->msg.start + sizeof(int) + sizeof(Type));
		  break;
		case TYPE_ARRAY:
		  int n_array;
		  parse_Len(&n_array, msg);
		  for(int i = 0; i < n_array; ++i)
		  {
		    Buffer key;
		    parse_Buffer(&key, msg);
		    printf("Buffer key: %.*s\n", key.n, key.start);
		  }
		  break;
		case TYPE_NONE:
		  printf("Type of response was not set.\n");
		  break;
	      }
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
