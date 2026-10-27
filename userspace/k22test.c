#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <linux/k22info.h>

#define K22TREE_SYSCALL 472
#define NSEC_PER_SEC 1000000000UL

static int find_parent(const int *stack, int top, int parent_pid)
{
	for (int depth = top; depth >= 0; depth--)
		if (stack[depth] == parent_pid)
			return depth;

	return -1;
}

int main(void)
{
	struct k22info *buf;
	int *stack;
	int capacity = 4;
	int ne;
	int top = 0;

	for (;;) {
		if ((size_t)capacity > SIZE_MAX / sizeof(*buf)) {
			fprintf(stderr, "process buffer size overflow\n");
			return EXIT_FAILURE;
		}

		buf = malloc((size_t)capacity * sizeof(*buf));
		if (!buf) {
			perror("malloc");
			return EXIT_FAILURE;
		}

		ne = capacity;
		printf("- User-space buf. size: %d\n", capacity);
		if (syscall(K22TREE_SYSCALL, buf, &ne) == 0) {
			printf("- syscall return val:   0 (size: %d)\n"
			       "--- OK ---\n\n", ne);
			break;
		}

		if (errno != ENOSPC) {
			perror("k22tree");
			free(buf);
			return EXIT_FAILURE;
		}

		printf("- syscall return val:   -1 (size: %d)\n",
		       capacity <= INT_MAX / 2 ? capacity * 2 : capacity);
		free(buf);
		if (capacity > INT_MAX / 2) {
			fprintf(stderr, "process count exceeds supported capacity\n");
			return EXIT_FAILURE;
		}
		capacity *= 2;
	}

	if (ne < 0 || ne > capacity) {
		fprintf(stderr, "k22tree returned an invalid entry count: %d\n", ne);
		free(buf);
		return EXIT_FAILURE;
	}

	stack = ne ? malloc((size_t)ne * sizeof(*stack)) : NULL;
	if (ne && !stack) {
		perror("malloc");
		free(buf);
		return EXIT_FAILURE;
	}

	printf("#comm,pid,ppid,fcldpid,nsblpid,nvcsw,nivcsw,stime,userpc,kernelpc\n");

	for (int i = 0; i < ne; i++) {
		int depth = 0;

		if (i > 0) {
			int parent_depth = find_parent(stack, top,
						       buf[i].parent_pid);

			if (parent_depth < 0 || parent_depth + 1 >= ne) {
				fprintf(stderr,
					"k22tree entries are not in parent-first DFS order\n");
				free(stack);
				free(buf);
				return EXIT_FAILURE;
			}
			depth = parent_depth + 1;
		}

		stack[depth] = buf[i].pid;
		top = depth;

		for (int j = 0; j < depth; j++)
			putchar('-');

		printf("%s,%d,%d,%d,%d,%lu,%lu,%lu,%p,%p\n",
		       buf[i].comm, buf[i].pid, buf[i].parent_pid,
		       buf[i].first_child_pid, buf[i].next_sibling_pid,
		       buf[i].nvcsw, buf[i].nivcsw,
		       buf[i].start_time / NSEC_PER_SEC,
		       (void *)(uintptr_t)buf[i].userpc,
		       (void *)(uintptr_t)buf[i].kernelpc);
	}

	free(stack);
	free(buf);
	return EXIT_SUCCESS;
}
