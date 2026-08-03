#include <stdlib.h>
#define ASSERT(condition) (condition ? 0 : (printf("Failed: %s", #condition), *(int *)0 =0))
#define BUFFER(str) ((Buffer){.start=(uint8_t *)(str), .n = sizeof(str) - 1})
#define N(arr) (sizeof(arr)/sizeof(arr[0]))

typedef struct
{
  uint8_t * start;
  uint32_t n;
} Buffer;

#define KEYVAL Buffer key; Buffer value

enum IO
{
  Read,
  Write,
};

enum DB_Action
{
  NONE,
  SET,
  GET,
  DELETE
};
const char* all_DB_Action[] = {"NONE", "SET", "GET", "DELETE"};

enum State
{
  None,
  Type,
  MsgLen,
  Msg,
  Send,
};

enum IO state_to_io[] = {
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
  uint8_t* start;
  enum State state;
} Message;


typedef struct
{
  Message msg;
  int fd;
} State;

void set_state_reading(State * s)
{
  s->msg.state = MsgLen;
  s->msg.n_byte = 4;
  s->msg.n_byte_processed = 0;
  s->msg.start = malloc(s->msg.n_byte);
}

void set_state_writing(State * s)
{
  s->msg.state = Send;
  s->msg.n_byte_processed = 0;
}

// mark state machine
enum State try_to_transition(State * s)
{
  enum State state = None;
  // todo when to free?
  if(s->msg.n_byte_processed == s->msg.n_byte)
  {
    enum State pre = s->msg.state;

    if(s->msg.state == MsgLen)
    {
      printf("Msg len: %d\n", *((int*)s->msg.start));
      s->msg.state = Msg;
      s->msg.n_byte = *((int*)s->msg.start);
      if(s->msg.n_byte == 0)
      {
	set_state_writing(s);
      }
      else
      {
	s->msg.n_byte_processed = 0;
	s->msg.start = malloc(s->msg.n_byte);
      }
    }
    else if(s->msg.state == Msg)
    {
      set_state_writing(s);
    }
    else if(s->msg.state == Send)
    {
      set_state_reading(s);
    }
    state = s->msg.state;
    printf("%d -> %d\n", pre, state);
  }
  return state;
}


void do_partial_io(State *state)
{
  enum IO operation = state_to_io[state->msg.state];
  uint8_t * offset = state->msg.start + state->msg.n_byte_processed;
  int n_bytes_to_process = state->msg.n_byte - state->msg.n_byte_processed;


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
    state->msg.error = 1;
  }
  else if (n_bytes_returned == -1)
  {
    perror(__FILE__);
    state->msg.error = 1;
  }
  else
  {
    state->msg.n_byte_processed += n_bytes_returned;
    printf("[%d / %d] bytes %s\n", state->msg.n_byte_processed, state->msg.n_byte, operation == Read ? "read" : "written");
  }
}

void add_byte(Message* msg, void* data, int n_byte_add)
{
  int n_bytes_updated = msg->n_byte + n_byte_add;
  uint8_t * new_buffer = malloc(n_bytes_updated);
  if(!new_buffer)
  {
    perror(__FILE__);
    exit(errno);
  }

  memcpy(new_buffer, msg->start, msg->n_byte);
  memcpy(new_buffer + msg->n_byte, data, n_byte_add);
  msg->start = new_buffer;
  msg->n_byte = n_bytes_updated;
}

void add_Buffer(Message* msg, Buffer buffer)
{
  add_byte(msg, &buffer.n, 4);
  add_byte(msg, buffer.start, buffer.n);
}
