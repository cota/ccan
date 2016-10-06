/* Licensed under BSD-MIT - see LICENSE file for details */
#ifndef CCAN_SLIST_H
#define CCAN_SLIST_H
//#define CCAN_SLIST_DEBUG 1
#include <ccan/str/str.h>
#include <ccan/container_of/container_of.h>
#include <ccan/check_type/check_type.h>

/**
 * struct slist_node - an entry in a singly-linked list
 * @next: next entry (self if empty)
 *
 * This is used as an entry in a singly-linked list.
 * Example:
 *	struct child {
 *		const char *name;
 *		// Linked list of all us children.
 *		struct slist_node slist;
 *	};
 */
struct slist_node {
	struct slist_node *next;
};

/**
 * struct slist_head - the head of a singly-linked list
 * @h: the slist_head (containing next pointer)
 *
 * This is used as the head of a singly-linked list.
 * Example:
 *	struct parent {
 *		const char *name;
 *		struct slist_head children;
 *		unsigned int num_children;
 *	};
 */
struct slist_head {
	struct slist_node n;
};

#define SLIST_LOC __FILE__  ":" stringify(__LINE__)
#ifdef CCAN_SLIST_DEBUG
#define slist_debug(h, loc) slist_check((h), loc)
#define slist_debug_node(n, loc) slist_check_node((n), loc)
#else
#define slist_debug(h, loc) ((void)loc, h)
#define slist_debug_node(n, loc) ((void)loc, n)
#endif

/**
 * SLIST_HEAD_INIT - initializer for an empty slist_head
 * @name: the name of the slist.
 *
 * Explicit initializer for an empty slist.
 *
 * See also:
 *	SLIST_HEAD, slist_head_init()
 *
 * Example:
 *	static struct slist_head my_slist = SLIST_HEAD_INIT(my_slist);
 */
#define SLIST_HEAD_INIT(name) { { &(name).n } }

/**
 * SLIST_HEAD - define and initialize an empty slist_head
 * @name: the name of the slist.
 *
 * The SLIST_HEAD macro defines a slist_head and initializes it to an empty
 * slist.  It can be prepended by "static" to define a static slist_head.
 *
 * See also:
 *	SLIST_HEAD_INIT, slist_head_init()
 *
 * Example:
 *	static SLIST_HEAD(my_global_slist);
 */
#define SLIST_HEAD(name) \
	struct slist_head name = SLIST_HEAD_INIT(name)

/**
 * slist_head_init - initialize a slist_head
 * @h: the slist_head to set to the empty slist
 *
 * Example:
 *	...
 *	struct parent *parent = malloc(sizeof(*parent));
 *
 *	slist_head_init(&parent->children);
 *	parent->num_children = 0;
 */
static inline void slist_head_init(struct slist_head *h)
{
	h->n.next = &h->n;
}

/**
 * slist_add - add an entry at the start of a linked list.
 * @h: the slist_head to add the node to
 * @n: the slist_node to add to the list.
 *
 * The slist_node does not need to be initialized; it will be overwritten.
 * Example:
 *	struct child *child = malloc(sizeof(*child));
 *
 *	child->name = "marvin";
 *	slist_add(&parent->children, &child->slist);
 *	parent->num_children++;
 */
#define slist_add(h, n) slist_add_(h, n, SLIST_LOC)
static inline void slist_add_(struct slist_head *h,
			      struct slist_node *n,
			      const char *abortstr)
{
	n->next = h->n.next;
	h->n.next = n;
}

/**
 * slist_for_each - iterate through a slist.
 * @h: the slist_head (warning: evaluated multiple times!)
 * @i: the structure containing the slist_node
 * @member: the slist_node member of the structure
 *
 * This is a convenient wrapper to iterate @i over the entire slist.  It's
 * a for loop, so you can break and continue as normal.
 *
 * Example:
 *	slist_for_each(&parent->children, child, slist)
 *		printf("Name: %s\n", child->name);
 */
#define slist_for_each(h, i, member)					\
	slist_for_each_off(h, i, slist_off_var_(i, member))

/**
 * slist_for_each_off - iterate through a slist of memory regions.
 * @h: the slist_head
 * @i: the pointer to a memory region wich contains slist node data.
 * @off: offset(relative to @i) at which slist node data resides.
 *
 * This is a low-level wrapper to iterate @i over the entire slist, used to
 * implement all oher, more high-level, for-each constructs. It's a for loop,
 * so you can break and continue as normal.
 *
 * WARNING! Being the low-level macro that it is, this wrapper doesn't know
 * nor care about the type of @i. The only assumption made is that @i points
 * to a chunk of memory that at some @offset, relative to @i, contains a
 * properly filled `struct slist_node' which in turn contains pointers to
 * memory chunks and it's turtles all the way down. With all that in mind
 * remember that given the wrong pointer/offset couple this macro will
 * happily churn all you memory untill SEGFAULT stops it, in other words
 * caveat emptor.
 *
 * It is worth mentioning that one of legitimate use-cases for that wrapper
 * is operation on opaque types with known offset for `struct slist_node'
 * member(preferably 0), because it allows you not to disclose the type of
 * @i.
 *
 * Example:
 *	slist_for_each_off(&parent->children, child,
 *				offsetof(struct child, slist))
 *		printf("Name: %s\n", child->name);
 */
#define slist_for_each_off(h, i, off)					\
	for (i = slist_node_to_off_(slist_debug(h, SLIST_LOC)->n.next,	\
					(off));				\
	     slist_node_from_off_((void *)i, (off)) != &(h)->n;		\
	     i = slist_node_to_off_(slist_node_from_off_((void *)i,	\
					(off))->next, (off)))

/* Offset helper functions so we only single-evaluate. */
static inline void *slist_node_to_off_(struct slist_node *node, size_t off)
{
	return (void *)((char *)node - off);
}
static inline struct slist_node *slist_node_from_off_(void *ptr, size_t off)
{
	return (struct slist_node *)((char *)ptr + off);
}

/* Get the offset of the member, but make sure it's a slist_node. */
#define slist_off_(type, member)					\
	(container_off(type, member) +					\
	 check_type(((type *)0)->member, struct slist_node))

#define slist_off_var_(var, member)				\
	(container_off_var(var, member) +			\
	 check_type(var->member, struct slist_node))

#if HAVE_TYPEOF
#define slist_typeof(var) typeof(var)
#else
#define slist_typeof(var) void *
#endif

#endif /* CCAN_SLIST_H */
