#include <stdlib.h>
#include "list.h"
#include "man_bomber.h"

int main()
{
	struct bomb b1;
	struct bomb b2;
	struct bomb b3;
	struct bomb *p1;
	struct bomb *p2;
	struct bomb *p3;
	struct list_node *head;
	int ret = 0;
	INIT_LIST_HEAD(&b1.node);
	INIT_LIST_HEAD(&b2.node);
	INIT_LIST_HEAD(&b3.node);
	head = &b1.node;
	b1.x = 1;
	b2.x = 2;
	b3.x = 3;
	b1.y = 9;
	b2.y = 8;
	b3.y = 7;

	list_add(&b2.node, &head);
	list_add(&b3.node, &head);
	p1 = list_entry(struct bomb, head, node);
	p2 = list_entry(struct bomb, head->next, node);
	p3 = list_entry(struct bomb, head->next->next, node);
	if (p1 == &b1 && p2 == &b2 && p3 == &b3)
		return 0;
	else
		return 1;
}
