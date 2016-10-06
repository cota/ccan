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
 * The SLIST_HEAD macro defines an slist_head and initializes it to an empty
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
 * slist_head_init - initialize an slist_head
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
 * slist_add_after - add an entry after an existing node in a linked slist
 * @h: the slist_head to add the node to (for debugging)
 * @p: the existing slist_node to add the node after
 * @n: the new slist_node to add to the slist.
 *
 * The existing slist_node must already be a member of the slist.
 * The new slist_node does not need to be initialized; it will be overwritten.
 *
 * Example:
 *	struct child c1, c2, c3;
 *	SLIST_HEAD(h);
 *
 *	slist_add(&h, &c1.slist);
 *	slist_add(&h, &c3.slist);
 *	slist_add_after(&h, &c1.slist, &c2.slist);
 */
#define slist_add_after(h, p, n) slist_add_after_(h, p, n, SLIST_LOC)
static inline void slist_add_after_(struct slist_head *h,
				    struct slist_node *p,
				    struct slist_node *n,
				    const char *abortstr)
{
	n->next = p->next;
	p->next = n;
	(void)slist_debug(h, abortstr);
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
 * slist_empty - is an slist empty?
 * @h: the slist_head
 *
 * If the slist is empty, returns true.
 *
 * Example:
 *	assert(slist_empty(&parent->children) == (parent->num_children == 0));
 */
#define slist_empty(h) slist_empty_(h, SLIST_LOC)
static inline bool slist_empty_(const struct slist_head *h, const char* abortstr)
{
	(void)slist_debug(h, abortstr);
	return h->n.next == &h->n;
}

/**
 * slist_top - get the first entry in an slist
 * @h: the slist_head
 * @type: the type of the entry
 * @member: the slist_node member of the type
 *
 * If the slist is empty, returns NULL.
 *
 * Example:
 *	struct child *first;
 *	first = slist_top(&parent->children, struct child, slist);
 *	if (!first)
 *		printf("Empty slist!\n");
 */
#define slist_top(h, type, member)					\
	((type *)slist_top_((h), slist_off_(type, member)))

static inline const void *slist_top_(const struct slist_head *h, size_t off)
{
	if (slist_empty(h))
		return NULL;
	return (const char *)h->n.next - off;
}

/**
 * slist_pop - remove the first entry in an slist
 * @h: the slist_head
 * @type: the type of the entry
 * @member: the slist_node member of the type
 *
 * If the slist is empty, returns NULL.
 *
 * Example:
 *	struct child *one;
 *	one = slist_pop(&parent->children, struct child, slist);
 *	if (!one)
 *		printf("Empty slist!\n");
 */
#define slist_pop(h, type, member)					\
	((type *)slist_pop_((h), slist_off_(type, member)))

static inline const void *slist_pop_(struct slist_head *h, size_t off)
{
	struct slist_node *n;

	if (slist_empty(h))
		return NULL;
	n = h->n.next;
	h->n.next = n->next;
	return (const char *)n - off;
}

/**
 * slist_for_each - iterate through an slist.
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
 * slist_next - get the next entry in a slist
 * @h: the slist_head
 * @i: a pointer to an entry in the slist.
 * @member: the slist_node member of the structure
 *
 * If @i was the last entry in the slist, returns NULL.
 *
 * Example:
 *	struct child *second;
 *	second = slist_next(&parent->children, first, slist);
 *	if (!second)
 *		printf("No second child!\n");
 */
#define slist_next(h, i, member)						\
	((slist_typeof(i))slist_entry_or_null(slist_debug(h,		\
					    __FILE__ ":" stringify(__LINE__)), \
					    (i)->member.next,		\
					    slist_off_var_((i), member)))

/**
 * slist_for_each_off - iterate through an slist of memory regions.
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

/* Get the offset of the member, but make sure it's an slist_node. */
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

/* Returns member, or NULL if at end of slist. */
static inline void *slist_entry_or_null(const struct slist_head *h,
				       const struct slist_node *n,
				       size_t off)
{
	if (n == &h->n)
		return NULL;
	return (char *)n - off;
}

#endif /* CCAN_SLIST_H */
