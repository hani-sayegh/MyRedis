#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "stddef.h"


#include "hashmap.h"

typedef struct  // string to string
{
  //data
  Buffer  key;
  Buffer value;
  //structure
  Node node;
} StringToString;


int check_equal_buffer(Buffer* b1, Buffer* b2)
{
  int result = b1->n == b2->n;
  for (int i = 0; result && i < b1->n; ++i)
  {
    result = b1->start[i] == b2->start[i];
  }
  return result;
}

int check_equal(Node* n1, Node* n2)
{
  StringToString* first = BASE(n1, StringToString, node);
  StringToString* second = BASE(n2, StringToString, node);

  return check_equal_buffer(&first->key, &second->key);
}

#include "hashmap.c"

#define ENTRY(KEY, VAL) ((StringToString){.key = BUFFER(KEY), .value = BUFFER(VAL), .node = {.code = hash(BUFFER(KEY))}})

#define N(arr) (sizeof(arr)/sizeof(arr[0]))

int main()
{
  printf("Hello\n");
  HMap map = create_hmap(check_equal);

  StringToString one = ENTRY("Hello", "World");
  StringToString two = ENTRY("Bye", "World");
  StringToString three = ENTRY("Incendiary", "To excite anger");

  insert(&map, &two.node);
  insert(&map, &three.node);
  insert(&map, &one.node);
  // delete(&map, &two.node);

  if(contains(&map, &one.node))
  {
    printf("found key\n");
  }
  else
  {
    printf("Key not found\n");
  }

  print_map(&map);
  return 0;
}
