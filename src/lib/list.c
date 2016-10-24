#include <stdlib.h>

#include "list.h"

void list_add(struct list_node *entry, struct list_node *head)
{
	struct list_node *cur = head;
	while (cur->next)
		cur = cur->next;
	cur->next = entry;
	entry->next = NULL;
}

void list_remove(struct list_node *entry, struct list_node **head)
{
	struct list_node **cur = head;
	while (*cur != entry)
		cur = &(*cur)->next;
	*cur = (*cur)->next;
}
