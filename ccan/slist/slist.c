/* Licensed under BSD-MIT - see LICENSE file for details */
#include <stdio.h>
#include <stdlib.h>
#include "slist.h"

#ifdef CCAN_SLIST_DEBUG
static void *corrupt(const char *abortstr,
		     const struct slist_node *head,
		     const struct slist_node *node,
		     unsigned int count)
{
	if (abortstr) {
		fprintf(stderr,
			"%s: prev corrupt in node %p (%u) of %p\n",
			abortstr, node, count, head);
		abort();
	}
	return NULL;
}

struct slist_node *slist_check_node(const struct slist_node *node,
				    const char *abortstr)
{
	const struct slist_node *p, *n;
	int count = 0;

	for (p = node, n = node->next; n != node; p = n, n = n->next) {
		count++;
		if (n->prev != p)
			return corrupt(abortstr, node, n, count);
	}
	/* Check prev on head node. */
	if (node->prev != p)
		return corrupt(abortstr, node, node, 0);

	return (struct slist_node *)node;
}
#else
struct slist_node *slist_check_node(const struct slist_node *node,
				    const char *abortstr)
{
	return (struct slist_node *)node;
}
#endif

struct slist_head *slist_check(const struct slist_head *h, const char *abortstr)
{
	if (!slist_check_node(&h->n, abortstr))
		return NULL;
	return (struct slist_head *)h;
}
