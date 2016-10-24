#ifndef _LIST_H_
#define _LIST_H_

#define list_entry(type, node, member) \
	((type *)((char *)(node)-(unsigned long)(&((type *)0)->member)))

#define INIT_LIST_HEAD(x) \
	(x)->next = NULL

struct list_node {
	struct list_node *next;
};

void list_add(struct list_node *entry, struct list_node *head);
void list_remove(struct list_node *entry, struct list_node **head);

#endif /* _LIST_H_ */
