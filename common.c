#include <stdlib.h>
#define Assert(condition) (condition ? 0 : (printf("Failed: %s", #condition), *(int *)0 =0))

enum IO
{
  Read,
  Write,
};

enum RequestState
{
  None,
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

void set_state_reading(State * s)
{
  s->request = MsgLen;
  s->n_bytes = 4;
  s->n_bytes_processed = 0;
  s->data = malloc(s->n_bytes);
}

void set_state_writing(State * s)
{
  s->request = Send;
  s->n_bytes_processed = 0;
}

enum RequestState try_to_transition(State * s)
{
  enum RequestState state = None;
  // when to free?
  if(s->n_bytes_processed == s->n_bytes)
  {
    if(s->request == MsgLen)
    {
      s->request = Msg;
      s->n_bytes = (int)s->data[0];
      s->n_bytes_processed = 0;
      s->data = malloc(s->n_bytes);
    }
    else if(s->request == Msg)
    {
      set_state_writing(s);
    }
    else if(s->request == Send)
    {
      set_state_reading(s);
    }
    state = s->request;
  }
  return state;
}


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
