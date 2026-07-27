typedef struct
{
  const uint8_t * start;
  uint32_t n;
} Buffer;

typedef struct Node Node;
struct Node
{
  Node * next;
  int code;
};

typedef int (*FuncEqual)(Node *, Node *);

typedef struct 
{
  int n;
  int capacity;
  Node ** start;
  FuncEqual eq;
} HMap;

#define BASE(Ptr, Type, Prop) ((Type*)(((uint8_t*)(Ptr)) - offsetof(Type, Prop)))
