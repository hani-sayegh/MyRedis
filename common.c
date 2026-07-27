#include <stdlib.h>
#define ASSERT(condition) (condition ? 0 : (printf("Failed: %s", #condition), *(int *)0 =0))

enum IO
{
  Read,
  Write,
};

enum Operation
{
  SET,
  GET
};

enum State
{
  None,
  Type,
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
  uint8_t *start;
  int n;
} Str;

Str create_large_string()
{
  Str s = {};
  s.n  = 32 << 0;
  s.start = malloc(s.n);
  for(int i = 0; i < s.n; ++i)
  {
    s.start[i] = 'a' + i;
  }
  return s;
}


Str create_key()
{
  Str s = {};
  s.n  = 4;
  s.start = (uint8_t*)"Hani";
  return s;
}

Str create_value()
{
  Str s = {};
  s.n  = 6;
  s.start = (uint8_t*)"Sayegh";
  return s;
}

Str create_string_with_delimeter()
{
  Str s = {};
  s.n  = 32 << 1;
  s.start = malloc(s.n);
  s.start[0] = '\0';
  for(int i = 1; i < s.n; ++i)
  {
    s.start[i] = 'a';
  }
  return s;
}

typedef struct
{
  int n_byte;
  int n_byte_processed;
  int error;
  int fd;
  uint8_t *data;
  enum State state;
} State;

// void set_state_type(State * s)
// {
//   s->state = Type;
//   s->n_byte = 4;
//   s->n_bytes_processed = 0;
//   s->data = malloc(s->n_byte);
// }

void set_state_reading(State * s)
{
  s->state = MsgLen;
  s->n_byte = 4;
  s->n_byte_processed = 0;
  s->data = malloc(s->n_byte);
}

void set_state_writing(State * s)
{
  s->state = Send;
  s->n_byte_processed = 0;
}

// mark state machine
enum State try_to_transition(State * s)
{
  enum State state = None;
  // todo when to free?
  if(s->n_byte_processed == s->n_byte)
  {
    enum State pre = s->state;

    if(s->state == MsgLen)
    {
      printf("Msg len: %d\n", *((int*)s->data));
      s->state = Msg;
      s->n_byte = *((int*)s->data);
      if(s->n_byte == 0)
      {
	set_state_writing(s);
      }
      else
      {
	s->n_byte_processed = 0;
	s->data = malloc(s->n_byte);
      }
    }
    else if(s->state == Msg)
    {
      set_state_writing(s);
    }
    else if(s->state == Send)
    {
      set_state_reading(s);
    }
    state = s->state;
    printf("%d -> %d\n", pre, state);
  }
  return state;
}


void do_partial_io(State *state)
{
  enum IO operation = bla_state[state->state];
  uint8_t * offset = state->data + state->n_byte_processed;
  int n_bytes_to_process = state->n_byte - state->n_byte_processed;


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
    state->n_byte_processed += n_bytes_returned;
    printf("[%d / %d] bytes %s\n", state->n_byte_processed, state->n_byte, operation == Read ? "read" : "written");
  }
}

void add_operation(State* s, enum Operation o)
{
  ASSERT(s->n_byte == 0);
  s->n_byte = sizeof(enum Operation);
  s->data = malloc(s->n_byte);
  memcpy(s->data, &o, s->n_byte);
}

void complete_state(State* s)
{
  int n_bytes_updated = s->n_byte + sizeof(s->n_byte);
  uint8_t * new_buffer = malloc(n_bytes_updated);
  if(!new_buffer)
  {
    perror(__FILE__);
    exit(errno);
  }
  memcpy(new_buffer, &(s->n_byte), sizeof(s->n_byte));
  memcpy(new_buffer + sizeof(s->n_byte), s->data, s->n_byte);
  s->data = new_buffer;
  s->n_byte = n_bytes_updated;
}

void add_data(State * s, Str str)
{
  int n_bytes_updated = s->n_byte + sizeof(int) + str.n;
  uint8_t * new_buffer = malloc(n_bytes_updated);
  if(!new_buffer)
  {
    perror(__FILE__);
    exit(errno);
  }

  if(s->data)
  {
    memcpy(new_buffer, s->data, s->n_byte);
  }

  memcpy(new_buffer + s->n_byte, &str.n, sizeof(int));
  memcpy(new_buffer + s->n_byte + 4, str.start, str.n);
  s->data = new_buffer;
  s->n_byte = n_bytes_updated;
}
