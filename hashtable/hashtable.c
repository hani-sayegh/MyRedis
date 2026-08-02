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

Node** contains(Map *map, Node* target) 
{
  size_t pos = target->code & (map->capacity - 1);
  Node* node_current = map->start[pos];

  Node** result = 0;
  while(node_current != 0 && !result)
  {
    if(map->eq(node_current, target))
    {
      result = map->start + pos;
    }
    else
    {
      node_current = node_current->next;
    }
  }
  return result;
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


void insert(Map *map, Node *node) {
  Node** exists = contains(map, node);
  if(exists)
  {
    *exists = node;
  }
  else
  {
    size_t pos = node->code & (map->capacity - 1);
    node->next = map->start[pos];
    map->start[pos] = node;
    ++map->n;
  }
}

