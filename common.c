#include <stdlib.h>
#define ASSERT(condition) (condition ? 0 : (printf("Failed: %s", #condition), *(int *)0 =0))
#define BUFFER(str) ((Buffer){.start=(uint8_t *)(str), .n = sizeof(str) - 1})

typedef struct
{
  uint8_t * start;
  uint32_t n;
} Buffer;

typedef struct
{
  Buffer key;
  Buffer value;
} KeyVal;

enum IO
{
  Read,
  Write,
};

enum DB_Action
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

Buffer create_large_string()
{
  Buffer s = {};
  s.n  = 32 << 0;
  s.start = malloc(s.n);
  for(int i = 0; i < s.n; ++i)
  {
    s.start[i] = 'a' + i;
  }
  return s;
}

Buffer create_key()
{
  Buffer s = {};
  s.n  = 4;
  s.start = (uint8_t*)"Hani";
  return s;
}

Buffer create_value()
{
  Buffer s = {};
  s.n  = 6;
  s.start = (uint8_t*)"Sayegh";
  return s;
}

Buffer create_string_with_delimeter()
{
  Buffer s = {};
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
  uint8_t* start;
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
  s->start = malloc(s->n_byte);
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
      printf("Msg len: %d\n", *((int*)s->start));
      s->state = Msg;
      s->n_byte = *((int*)s->start);
      if(s->n_byte == 0)
      {
	set_state_writing(s);
      }
      else
      {
	s->n_byte_processed = 0;
	s->start = malloc(s->n_byte);
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
  uint8_t * offset = state->start + state->n_byte_processed;
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

void add_byte(State * s, void* data, int n_byte_add)
{
  int n_bytes_updated = s->n_byte + n_byte_add;
  uint8_t * new_buffer = malloc(n_bytes_updated);
  if(!new_buffer)
  {
    perror(__FILE__);
    exit(errno);
  }

  memcpy(new_buffer, s->start, s->n_byte);
  memcpy(new_buffer + s->n_byte, data, n_byte_add);
  s->start = new_buffer;
  s->n_byte = n_bytes_updated;
}

void add_Buffer(State* s, Buffer buffer)
{
  add_byte(s, &buffer.n, 4);
  add_byte(s, buffer.start, buffer.n);
}
