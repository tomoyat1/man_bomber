#include "list.h"
#include "man_bomber.h"

extern int bomb_equals_node_by_coords(struct bomb *b, struct list_node *n);

int main()
{
	struct bomb bombs[5];
	bombs[0].x = 10;
	bombs[0].y = 5;

	bombs[1].x = 10;
	bombs[1].y = 5;

	bombs[2].x = 10;
	bombs[2].y = 4;
	
	bombs[3].x = 9;
	bombs[3].y = 5;

	bombs[4].x = 9;
	bombs[4].y = 4;

	if (!bomb_equals_node_by_coords(&bombs[0], &bombs[1].node))
		return 1;
	if (bomb_equals_node_by_coords(&bombs[0], &bombs[2].node))
		return 1;
	if (bomb_equals_node_by_coords(&bombs[0], &bombs[3].node))
		return 1;
	if (bomb_equals_node_by_coords(&bombs[0], &bombs[4].node))
		return 1;
	return 0;


}
