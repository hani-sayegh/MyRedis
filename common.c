#include <stdlib.h>
#define Assert(condition) (condition ? 0 : (printf("Failed: %s", #condition), *(int *)0 =0))

enum IO
{
  Read,
  Write,
};

enum RequestState
{
  MsgLen,
  Msg,
  Send,
};

enum IO bla_state[] = {
  [MsgLen]  = Read,
  [Msg]     = Read,
  [Send]= Write,
};

typedef struct
{
  int n_bytes;
  int n_bytes_processed;
  int error;
  int fd;
  uint8_t *data;
  enum RequestState request;
} State;


void do_partial_io(State *state)
{
  enum IO operation = bla_state[state->request];
  uint8_t * offset = state->data + state->n_bytes_processed;
  int n_bytes_to_process = state->n_bytes - state->n_bytes_processed;


  int n_bytes_returned = 0;

  if(operation == Read)
  {
    n_bytes_returned = read(state->fd, offset, n_bytes_to_process);
  }
  else if(operation == Write)
  {
    n_bytes_returned = write(state->fd, offset, n_bytes_to_process);
  }

  if(n_bytes_returned == 0)
  {
    printf("EOF\n");
    state->error = 1;
  }
  else if (n_bytes_returned == -1)
  {
    perror(__FILE__);
    state->error = 1;
  }
  else
  {
    state->n_bytes_processed += n_bytes_returned;
    printf("[%d / %d] bytes %s\n", state->n_bytes_processed, state->n_bytes, operation == Read ? "read" : "written");
  }
}
