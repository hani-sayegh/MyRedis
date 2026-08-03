#define MAP_CAPACITY 1 << 10

Map create_map(FuncEqual eq)
{
  Map result = {};
  result.eq = eq;
  result.capacity = MAP_CAPACITY;
  result.start = calloc(result.capacity, sizeof(Node *));
  return result;
}

uint32_t hash(Buffer buffer)
{
  uint32_t h = 0x811C9DC5;
  for (size_t i = 0; i < buffer.n; i++) {
    h = (h + buffer.start[i]) * 0x01000193;
  }
  return h;
}

Node* delete(Map *map, Node* node) {
  size_t pos = node->code & (map->capacity - 1);
  Node** node_indirect = &map->start[pos];
  Node* deleted = 0;
  while(*node_indirect && !deleted)
  {
    if(map->eq(node, *node_indirect))
    {
      deleted = *node_indirect;
      *node_indirect = (*node_indirect)->next;
      --map->n;
    }
    else
    {
      node_indirect = &((*node_indirect)->next);
    }
  }
  return deleted;
}

void insert(Map* map, Node* node) 
{
  size_t pos = node->code & (map->capacity - 1);
  Node** node_indirect = map->start + pos;
  while(*node_indirect && !map->eq(*node_indirect, node))
  {
    node_indirect = &((*node_indirect)->next);
  }
  if(*node_indirect == 0)
  {
    *node_indirect = node;
    ++map->n;
  }
}

