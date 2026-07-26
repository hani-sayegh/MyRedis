#define MAP_CAPACITY 1 << 10

HMap create_hmap(FuncEqual eq)
{
  HMap result = {};
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

Node** contains(HMap *map, Node* target) 
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

int delete(HMap *map, Node* node) {
  size_t pos = node->code & (map->capacity - 1);
  Node** node_indirect = &map->start[pos];
  int deleted = 0;

  while(*node_indirect != 0 && !deleted)
  {
    Node * next = (*node_indirect)->next;
    if(map->eq(node, *node_indirect))
    {
      *node_indirect = next;
      deleted = 1;
    }
    else
    {
      node_indirect = &next;
    }
  }
  --map->n;
  return deleted;
}


void insert(HMap *map, Node *node) {
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
