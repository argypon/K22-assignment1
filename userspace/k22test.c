#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>
#include <linux/k22info.h>

int main(void)
{
	struct k22info *buf;
	int size = 4;
	int ne;
	int *stack;
	int top = 0;

	/* try again with a bigger buffer until everything fits */
	while (1) {
		buf = malloc(size * sizeof(struct k22info));
		if (!buf) {
			perror("malloc");
			return 1;
		}

		ne = size;
		printf("- User-space buf. size: %d\n", size);

		if (syscall(472, buf, &ne) == 0) {
			printf("- syscall return val:   0 (size: %d)\n--- OK ---\n\n", ne);
			break;
		}
		if (errno != ENOSPC) {
			perror("k22tree");
			return 1;
		}

		printf("- syscall return val:   -1 (size: %d)\n", size * 2);
		free(buf);
		size *= 2;
	}

	/* stack[d] = pid of the last process printed at depth d */
	stack = malloc(ne * sizeof(int));
	if (!stack) {
		perror("malloc");
		return 1;
	}

	printf("#comm,pid,ppid,fcldpid,nsblpid,nvcsw,nivcsw,stime,userpc,kernelpc\n");

	for (int i = 0; i < ne; i++) {
		int depth = 0;

		/* the depth is the depth of the parent plus one */
		if (i > 0) {
			for (int d = top; d >= 0; d--) {
				if (stack[d] == buf[i].parent_pid) {
					depth = d + 1;
					break;
				}
			}
		}
		stack[depth] = buf[i].pid;
		top = depth;

		for (int j = 0; j < depth; j++)
			printf("-");

		printf("%s,%d,%d,%d,%d,%lu,%lu,%lu,%p,%p\n",
		       buf[i].comm, buf[i].pid, buf[i].parent_pid,
		       buf[i].first_child_pid, buf[i].next_sibling_pid,
		       buf[i].nvcsw, buf[i].nivcsw, buf[i].start_time,
		       (void *)buf[i].userpc, (void *)buf[i].kernelpc);
	}

	free(buf);
	free(stack);
	return 0;
}