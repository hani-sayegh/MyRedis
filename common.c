#include <stdlib.h>
typedef struct
{
int n_bytes;
int fd;
uint8_t *data;
} State;

void write_partial(State * state)
{
    int bytes_written = write(state->fd, state->data, state->n_bytes);
    if (bytes_written == 0)
    {
        printf("EOF");
    }
    else if(bytes_written == -1)
    {

        perror(__FILE__);
    }
    else
    {
      printf("[%d / %d] bytes sent\n", bytes_written, state->n_bytes);
      state->n_bytes -= bytes_written;
    }
}

void read_partial(State *state)
{
    int bytes_read = read(state->fd, state->data, state->n_bytes);
    if(bytes_read == 0)
    {
        printf("EOF");
    }
    if (bytes_read == -1)
    {
        perror(__FILE__);
    }
    else
    {
      printf("[%d / %d] bytes read\n", bytes_read, state->n_bytes);
      state->n_bytes -= bytes_read;
    }
}
