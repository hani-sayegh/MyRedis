#include "stdio.h"
#include "string.h"
#include "errno.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "unistd.h"
#include "signal.h"
#include "poll.h"


#include "common.c"
#include "./hashtable/hashtable.h"
#include "./hashtable/hashtable.c"

#define MAX_CONN 1024 
#define uint8_t Byte


typedef struct
{

  // struct pollfd {
  //     int   fd;         /* file descriptor */
  //     short events;     /* requested events */
  //     short revents;    /* returned events */
  // };
  struct pollfd start [MAX_CONN];
  State state[MAX_CONN];
  int n;
} ArrayInt;

typedef struct 
{
  Node node;
  KeyVal data; 
} DbEntry;

int check_Str_equal(Buffer* a, Buffer* b)
{
  int result = a->n == b->n;
  for(int i = 0; i < a->n && result; ++i)
  {
    if(a->start[i] != b->start[i])
    {
      result = 0;
    }
  }
  return result;
}

int equal(Node* a, Node* b)
{
  return check_Str_equal(&((DbEntry*)a)->data.key, &((DbEntry*)b)->data.key);
}

void print_map(Map* map)
{
  if(map->n)
  {
    for(int idx_slot = 0; idx_slot < map->capacity; ++idx_slot)
    {
      Node* head = map->start[idx_slot];
      int idx_chain = 0;
      while(head)
      {
	DbEntry* e = (DbEntry*)head;
	printf("[%d][%d] [%.*s, %.*s]\n", idx_slot, idx_chain, e->data.key.n, e->data.key.start, e->data.value.n, e->data.value.start);
	head = head->next;
	++idx_slot;
      }
    }
  }
  else
  {
    printf("Map Empty\n");
  }
}

int main()
{
  struct sigaction ignore = {};
  ignore.sa_handler       = SIG_IGN;
  sigaction(SIGPIPE, &ignore, 0);
  int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
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

	ArrayInt all_socket           = {};
	all_socket.start[all_socket.n].fd     = fd;

	//POLLIN There is data to read.
	// POLLOUT
	//        Writing is now possible, though a write larger than the available space in a socket or pipe will still block (unless O_NONBLOCK is set).
	all_socket.start[all_socket.n].events = POLLIN;
	++all_socket.n;

	Map db = create_map(equal);
	while (1)
	{
	  printf("polling...\n");

	  //int poll(struct pollfd *fds, nfds_t nfds, int timeout);
	  err = poll(all_socket.start, all_socket.n, -1);
	  if (err == -1)
	  {
	    perror(__FILE__);
	  }
	  else
	  {
	    ASSERT(all_socket.n < MAX_CONN);
	    int n = all_socket.n;
	    for (int idx_socket = 0; idx_socket < n; ++idx_socket)
	    {
	      int revents = all_socket.start[idx_socket].revents;
	      if (revents)
	      {
		if (idx_socket == 0)
		{
		  struct sockaddr_in client_addr = {};
		  socklen_t addrlen              = sizeof(client_addr);

		  int fd_client = accept(all_socket.start[0].fd, (struct sockaddr *)&client_addr, &addrlen);
		  if (fd_client == -1)
		  {
		    perror(__FILE__);
		  }
		  else
		  {
		    uint32_t ip = client_addr.sin_addr.s_addr;
		    fprintf(stderr, "[%d]  from %u.%u.%u.%u:%u\n", fd_client,
			ip & 255, (ip >> 8) & 255, (ip >> 16) & 255,
			ip >> 24, ntohs(client_addr.sin_port));

		    all_socket.start[n].fd     = fd_client;
		    all_socket.start[n].events = POLLIN;

		    all_socket.state[n].fd     = fd_client;
		    all_socket.state[n].msg.error     = 0;
		    set_state_reading(&all_socket.state[n]);

		    ++all_socket.n;
		  }
		}
		else
		{
		  // mark handle client state
		  State * s = all_socket.state + idx_socket;
		  struct pollfd* poll_stuff = all_socket.start + idx_socket;
		  do_partial_io(s);

		  if(try_to_transition(s) == Send)
		  {
		    struct
		    {
		      enum DB_Action type;
		      DbEntry* db_entry;
		    } command = {.db_entry = calloc(1, sizeof(DbEntry))};

		    int n_bytes_parsed = 0;
		    if(s->msg.n_byte - n_bytes_parsed < 4)
		    {
		      abort(); 
		    }
		    command.type = *(int*)(s->msg.start + n_bytes_parsed);
		    n_bytes_parsed += 4;
		    if(s->msg.n_byte - n_bytes_parsed < 4)
		    {
		      abort(); 
		    }
		    command.db_entry->data.key.n = *(int*)(s->msg.start + n_bytes_parsed);
		    n_bytes_parsed += 4;
		    if(s->msg.n_byte - n_bytes_parsed < command.db_entry->data.key.n)
		    {
		      abort(); 
		    }
		    command.db_entry->data.key.start = malloc(command.db_entry->data.key.n);
		    memcpy(command.db_entry->data.key.start, s->msg.start + n_bytes_parsed, command.db_entry->data.key.n);
		    n_bytes_parsed += command.db_entry->data.key.n;
		    if(s->msg.n_byte - n_bytes_parsed < 4)
		    {
		      abort();
		    }
		    command.db_entry->data.value.n = *(int*)(s->msg.start + n_bytes_parsed);
		    n_bytes_parsed += 4;
		    if(s->msg.n_byte - n_bytes_parsed < command.db_entry->data.value.n)
		    {
		      abort(); 
		    }
		    command.db_entry->data.value.start = malloc(command.db_entry->data.value.n);
		    memcpy(command.db_entry->data.value.start, s->msg.start + n_bytes_parsed, command.db_entry->data.value.n);
		    n_bytes_parsed += command.db_entry->data.value.n;

		    switch(command.type)
		    {
		      case NONE:
			break;
		      case SET:
			command.db_entry->node.code = hash(command.db_entry->data.key);
			insert(&db, &command.db_entry->node);
			break;
		      case GET:
			break;
		      case DELETE:
			command.db_entry->node.code = hash(command.db_entry->data.key);
			// lifetime: arbitrary
			// owner: caller
			Node* deleted = delete(&db, &command.db_entry->node);
			if(deleted)
			{
			  free((DbEntry*)deleted);
			}
			break;
		      default:
			abort();
			break;
		    }

		    print_map(&db);
		    poll_stuff->events = POLLOUT;
		    char msg [] = "Hello from Hani's server!!";
		    int n_data = sizeof(msg) - 1;
		    s->msg.n_byte = sizeof(int) + n_data;
		    s->msg.start = malloc(s->msg.n_byte);

		    memcpy(s->msg.start, &n_data, sizeof(n_data));
		    memcpy(s->msg.start + 4, msg, n_data);
		  }
		  else
		  {
		    poll_stuff->events = POLLIN;
		  }
		}
	      }
	    }

	    // clean up
	    for(int idx_fd = 1; idx_fd < all_socket.n; ++idx_fd)
	    {
	      if(all_socket.state[idx_fd].msg.error)
	      {
		int fd = all_socket.state[idx_fd].fd;
		printf("[%d] removed\n", fd);
		if(close(fd) == -1)
		{
		  perror(__FILE__);
		}
		--all_socket.n;
		all_socket.state[idx_fd] = all_socket.state[all_socket.n];
	      }
	    }
	  }
	}
      }
    }
  }
  printf("Exiting\n");
  return 0;
}
