#include "stdio.h"
#include "string.h"
#include "errno.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "unistd.h"
#include "signal.h"
#include "poll.h"

#include "common.c"
#define MAX_CONN 1024 

#define Assert(condition) (condition ? 0 : (printf("Failed: %s", #condition), *(int *)0 =0))

typedef struct
{

  // struct pollfd {
  //     int   fd;         /* file descriptor */
  //     short events;     /* requested events */
  //     short revents;    /* returned events */
  // };
  struct pollfd start [MAX_CONN];
  State all_state[MAX_CONN];
  int n;
} ArrayInt;

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

	while (1)
	{
	  printf("Waiting for poll\n");

	  //int poll(struct pollfd *fds, nfds_t nfds, int timeout);
	  err = poll(all_socket.start, all_socket.n, -1);
	  if (err == -1)
	  {
	    perror(__FILE__);
	  }
	  else
	  {
	    Assert(all_socket.n < MAX_CONN);
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
		    all_socket.all_state[n].fd = fd_client;
		    all_socket.all_state[n].n_bytes = 4;
		    all_socket.all_state[n].data = calloc(1024, 1);

		    ++all_socket.n;
		  }
		}
		else
		{
		  // process_payload(fd_curr);
		  // char message_to_client[] = "The server sent you
		  // message!\n"; send_payload(fd_curr, message_to_client,
		  //              strlen(message_to_client));
		  // close(fd_curr);

		  read_partial(&all_socket.all_state[idx_socket]);

		  if(all_socket.all_state[idx_socket].n_bytes == 0)
		  {
		  }

		  printf("Read %d\n", *all_socket.all_state[idx_socket].data);


		}
	      }
	    }

	    // clean up
	    for(int idx_fd = 1; idx_fd < all_socket.n; ++idx_fd)
	    {
	      if(all_socket.all_state[idx_fd].n_bytes == 0)
	      {
		int fd = all_socket.all_state[idx_fd].fd;
		printf("[%d] removed\n", fd);
		if(close(fd) == -1)
		{
		  perror(__FILE__);
		}
		--all_socket.n;
		all_socket.all_state[idx_fd] = all_socket.all_state[all_socket.n];
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
