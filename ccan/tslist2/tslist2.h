/* Licensed under LGPL - see LICENSE file for details */
#ifndef CCAN_TSLIST2_H
#define CCAN_TSLIST2_H
#include <ccan/slist/slist.h>
#include <ccan/tcon/tcon.h>

/**
 * TSLIST2 - declare a typed list type (struct tslist)
 * @etype: the type the list will contain
 * @link: the name of the member of @etype that is the link
 *
 * This declares an anonymous structure to use for lists containing this type.
 * The actual list can be accessed using tslist2_raw().
 *
 * Example:
 *	#include <ccan/tslist2/tslist2.h>
 *	struct child {
 *		const char *name;
 *		struct slist_node list;
 *	};
 *	struct parent {
 *		const char *name;
 *		TSLIST2(struct child, list) children;
 *		unsigned int num_children;
 *	};
 *
 */
#define TSLIST2(etype, link)				\
	TCON_WRAP(struct slist_head,			\
		TCON_CONTAINER(canary, etype, link))

/**
 * TSLIST2_INIT - initalizer for an empty tslist
 * @name: the name of the list.
 *
 * Explicit initializer for an empty list.
 *
 * See also:
 *	tslist2_init()
 *
 * Example:
 *	TSLIST2(struct child, list) my_list = TSLIST2_INIT(my_list);
 */
#define TSLIST2_INIT(name) TCON_WRAP_INIT( SLIST_HEAD_INIT(*tcon_unwrap(&(name))) )

/**
 * tslist2_init - initialize a tslist
 * @h: the tslist to set to the empty list
 *
 * Example:
 *	...
 *	struct parent *parent = malloc(sizeof(*parent));
 *
 *	tslist2_init(&parent->children);
 *	parent->num_children = 0;
 */
#define tslist2_init(h) slist_head_init(tcon_unwrap(h))

/**
 * tslist2_raw - unwrap the typed list and check the type
 * @h: the tslist
 * @expr: the expression to check the type against (not evaluated)
 *
 * This macro usually causes the compiler to emit a warning if the
 * variable is of an unexpected type.  It is used internally where we
 * need to access the raw underlying list.
 */
#define tslist2_raw(h, expr) tcon_unwrap(tcon_container_check_ptr(h, canary, expr))

/**
 * tslist2_unwrap - unwrap the typed list without any checks
 * @h: the tslist
 */
#define tslist2_unwrap(h) tcon_unwrap(h)

/**
 * tslist2_add - add an entry at the start of a linked list.
 * @h: the tslist to add the node to
 * @n: the entry to add to the list.
 *
 * The entry's slist_node does not need to be initialized; it will be
 * overwritten.
 * Example:
 *	struct child *child = malloc(sizeof(*child));
 *
 *	child->name = "marvin";
 *	tslist2_add(&parent->children, child);
 *	parent->num_children++;
 */
#define tslist2_add(h, n) slist_add(tslist2_raw((h), (n)), tcon_member_of(h, canary, n))

/**
 * tslist2_add_tail - add an entry after an existing node in a tslist
 * @h: the tslist to add the node to
 * @p: the existing entry to add after
 * @n: the new entry to add to the slist
 *
 * The slist_node does not need to be initialized; it will be overwritten.
 * Example:
 *	struct child c1, c2, c3;
 *
 *	tslist2_add(&parent->children, c1);
 *	tslist2_add(&parent->children, c3);
 *	tslist2_add_after(&parent->children, c1, c2);
 */
#define tslist2_add_after(h, p, n)					\
	slist_add_after(tslist2_raw((h), (n)), tcon_member_of((h), canary, (p)), tcon_member_of((h), canary, (n)))

/**
 * tslist2_del_from - delete an entry from a linked list.
 * @h: the tslist @n is in
 * @n: the entry to delete
 *
 * This explicitly indicates which list a node is expected to be in,
 * which is better documentation and can catch more bugs.
 *
 * Note that this leaves @n->@member in an undefined state; it
 * can be added to another list, but not deleted again.
 *
 * Example:
 *	tslist2_del_from(&parent->children, child);
 *	parent->num_children--;
 */
#define tslist2_del_from(h, n) \
	slist_del_from(tslist2_raw((h), (n)), tcon_member_of((h), canary, (n)))

/**
 * tslist2_empty - is a list empty?
 * @h: the tslist
 *
 * If the list is empty, returns true.
 *
 * Example:
 *	assert(tslist2_empty(&parent->children) == (parent->num_children == 0));
 */
#define tslist2_empty(h) slist_empty(tcon_unwrap(h))

/**
 * tslist2_top - get the first entry in a list
 * @h: the tslist
 *
 * If the list is empty, returns NULL.
 *
 * Example:
 *	struct child *first;
 *	first = tslist2_top(&parent->children);
 *	if (!first)
 *		printf("Empty list!\n");
 */
#define tslist2_top(h) tcon_container_of((h), canary, slist_top_(tcon_unwrap(h), 0))

/**
 * tslist2_pop - remove the first entry in a list
 * @h: the tslist
 *
 * If the list is empty, returns NULL.
 *
 * Example:
 *	struct child *one;
 *	one = tslist2_pop(&parent->children);
 *	if (!one)
 *		printf("Empty list!\n");
 */
#define tslist2_pop(h) tcon_container_of((h), canary, slist_pop_(tcon_unwrap(h), 0))

/**
 * tslist2_for_each - iterate through a list.
 * @h: the tslist
 * @i: an iterator of suitable type for this list.
 *
 * This is a convenient wrapper to iterate @i over the entire list.  It's
 * a for loop, so you can break and continue as normal.
 *
 * Example:
 *	tslist2_for_each(&parent->children, child)
 *		printf("Name: %s\n", child->name);
 */
#define tslist2_for_each(h, i)					\
	slist_for_each_off(tslist2_raw((h), (i)), (i), tcon_offset((h), canary))

#if 0
/**
 * tslist2_for_each_safe - iterate through a list, maybe during deletion
 * @h: the tslist
 * @i: an iterator of suitable type for this list.
 * @nxt: another iterator to store the next entry.
 *
 * This is a convenient wrapper to iterate @i over the entire list.  It's
 * a for loop, so you can break and continue as normal.  The extra variable
 * @nxt is used to hold the next element, so you can delete @i from the list.
 *
 * Example:
 *	struct child *next;
 *	tslist2_for_each_safe(&parent->children, child, next) {
 *		tslist2_del_from(&parent->children, child);
 *		parent->num_children--;
 *	}
 */
#define tslist2_for_each_safe(h, i, nxt)				\
	slist_for_each_safe_off(tslist2_raw((h), (i)), (i), (nxt), tcon_offset((h), canary))
#endif

#endif /* CCAN_TSLIST2_H */
