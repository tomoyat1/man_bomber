#include <stdlib.h>

#include "list.h"

void list_add(struct list_node *entry, struct list_node **head)
{
	struct list_node **cur = head;
	while (*cur)
		cur = &(*cur)->next;
	*cur = entry;
	entry->next = NULL;
}

int list_count(struct list_node **head)
{
	int cnt;
	struct list_node *cur;
	cnt = 0;
	cur = *head;
	while (cur) {
		cnt++;
		cur = cur->next;
	}
	return cnt;
}

void list_remove(struct list_node *entry, struct list_node **head)
{
	struct list_node **cur = head;
	while (*cur != entry) {
		cur = &(*cur)->next;
		if (!*cur)
			return;
	}
	*cur = (*cur)->next;
}
