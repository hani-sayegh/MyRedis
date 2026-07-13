int write_full(int fd, void *data, int bytes)
{
  int n  = bytes;
  int gd = 1;
  while (n && gd)
  {
    int err = write(fd, data, bytes);
    if (err == -1)
    {
      gd = 0;
      perror(__FILE__);
    }
    else
    {
      n -= err;
      printf("[%d / %d] bytes sent\n", err, bytes);
    }
  }
  return gd;
}

int read_full(int fd, void *dest, int bytes)
{
  int n  = bytes;
  int gd = 1;
  while (n && gd)
  {
    int err = read(fd, dest, bytes);
    if (err == -1)
    {
      gd = 0;
      perror(__FILE__);
    }
    else
    {
      n -= err;
      printf("[%d / %d] bytes read\n", err, bytes);
    }
  }
  return gd;
}

void process_payload(int fd)
{
  int msg_len = 0;
  int gd      = read_full(fd, &msg_len, sizeof(msg_len));
  if (gd)
  {
    char msg[1000] = {};
    gd             = read_full(fd, &msg, msg_len);
    if (gd)
    {
      printf("Message: %s\n", msg);
    }
    else
    {
    }
  }
  else
  {
  }
}

void send_payload(int fd, char *msg, int byte)
{
  write_full(fd, &byte, sizeof(byte));
  write_full(fd, msg, byte);
}
