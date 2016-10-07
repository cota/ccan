#include <ccan/slist/slist.h>
#include <ccan/tap/tap.h>

#include <ccan/slist/slist.c>

struct child {
	const char *name;
	struct slist_node slist;
};

struct parent {
	const char *name;
	struct slist_head children;
	unsigned int num_children;
};

static SLIST_HEAD(static_slist);

int main(int argc, char *argv[])
{
	struct parent parent;
	struct child c1, c2, c3, *c;
	unsigned int i;
	struct slist_head slist = SLIST_HEAD_INIT(slist);
//	opaque_t *q, *nq;
//	struct slist_head opaque_slist = SLIST_HEAD_INIT(opaque_slist);

	plan_tests(31);
	/* Test SLIST_HEAD, SLIST_HEAD_INIT, slist_empty and check_slist */
	ok1(slist_empty(&static_slist));
	ok1(slist_check(&static_slist, NULL));
	ok1(slist_empty(&slist));
	ok1(slist_check(&slist, NULL));

	parent.num_children = 0;
	slist_head_init(&parent.children);
	/* Test slist_head_init */
	ok1(slist_empty(&parent.children));
	ok1(slist_check(&parent.children, NULL));
	ok1(slist_top(&parent.children, struct child, slist) == NULL);
	ok1(slist_pop(&parent.children, struct child, slist) == NULL);

	c3.name = "c3";
	slist_add(&parent.children, &c3.slist);
	/* Test slist_add and !slist_empty. */
	ok1(!slist_empty(&parent.children));
	ok1(c3.slist.next == &parent.children.n);
	ok1(parent.children.n.next == &c3.slist);
	/* Test slist_check */
	ok1(slist_check(&parent.children, NULL));

	c2.name = "c2";
	slist_add(&parent.children, &c2.slist);
	/* Test slist_add and !slist_empty. */
	ok1(!slist_empty(&parent.children));
	ok1(c3.slist.next == &parent.children.n);
	ok1(parent.children.n.next == &c2.slist);
	/* Test slist_check */
	ok1(slist_check(&parent.children, NULL));

	c1.name = "c1";
	slist_add(&parent.children, &c1.slist);
	/* Test slist_add and !slist_empty. */
	ok1(!slist_empty(&parent.children));
	ok1(c3.slist.next == &parent.children.n);
	ok1(parent.children.n.next == &c1.slist);
	ok1(c1.slist.next == &c2.slist);
	/* Test slist_check */
	ok1(slist_check(&parent.children, NULL));

	/* Test slist_check_node */
	ok1(slist_check_node(&c1.slist, NULL));
	ok1(slist_check_node(&c2.slist, NULL));
	ok1(slist_check_node(&c3.slist, NULL));

	/* Test slist_top */
	ok1(slist_top(&parent.children, struct child, slist) == &c1);

	/* Test slist_pop */
	ok1(slist_pop(&parent.children, struct child, slist) == &c1);
	ok1(slist_top(&parent.children, struct child, slist) == &c2);
	slist_add(&parent.children, &c1.slist);

	/* Test slist_for_each. */
	i = 0;
	slist_for_each(&parent.children, c, slist) {
		switch (i++) {
		case 0:
			ok1(c == &c1);
			break;
		case 1:
			ok1(c == &c2);
			break;
		case 2:
			ok1(c == &c3);
			break;
		}
		if (i > 2)
			break;
	}
	ok1(i == 3);

	return exit_status();
}
