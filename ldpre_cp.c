// gcc -shared -o barcp.so -ldl $0
// LD_PRELOAD=./barcp.so cp <source> <dest>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
// for RTLD_NEXT
#define __USE_GNU
// dlsym
#include <dlfcn.h>

struct gg
{
	int fd[2];
	off_t sz[2];
	off_t szl;
	time_t tlastp;
	int er;
};

struct gg *__global_fds = NULL;

typedef ssize_t (*read_t) (int, void*, size_t);
typedef ssize_t (*write_t) (int, const void*, size_t);

read_t true_read;
write_t true_write;


inline void
__lib_init ()
{
	long sys; // return from sysconf ()
	if (!__global_fds)
	{
		//fprintf (stderr, "OK\n");
		if (!(true_read = (read_t)dlsym (RTLD_NEXT, "read")))
			return;
		if (!(true_write = (write_t)dlsym (RTLD_NEXT, "write")))
			return;
		sys = sysconf (_SC_OPEN_MAX);
		if (sys >= 2) // gary mooo ^_^
		{
			if ((__global_fds = (struct gg*)calloc (1, sizeof(struct gg))))
			{
				__global_fds->fd[0] = -1;
				__global_fds->fd[1] = -1;
			}
		}
	}
}

ssize_t write (int fd, const void *buf, size_t size)
{	
	__lib_init ();
	ssize_t ret = 0;
	struct stat sstat;
	clock_t temp;
	if (true_write)
	{
		if ((fd != STDOUT_FILENO && fd != STDERR_FILENO) && __global_fds)
		{
			// init first
			if (__global_fds->fd[1] != fd)
			{
				__global_fds->fd[1] = fd;
				fstat(fd, &sstat);
				__global_fds->sz[1] = sstat.st_size;
			}
		}
		ret = true_write (fd, buf, size);
		if ((fd != STDOUT_FILENO && fd != STDERR_FILENO) && __global_fds)
		{
			// count
			if (__global_fds->sz[1] > __global_fds->sz[0] || !(__global_fds->sz[0]))
			{
				if (!__global_fds->er)
				{
					fprintf (stdout, "\n WARNING: size out > size in or size in == 0.");
					fprintf (stdout, " (in:%ld, out:%ld)\n", __global_fds->sz[0], __global_fds->sz[1]);
					fflush (stdout);
					__global_fds->er = 1;
				}
			}
			else
			if (!ret)
			{
				fprintf (stdout, " WARNING: write () return zero\n");
			}
			else
			{
				__global_fds->sz[1] += ret;
				//   если 
				// ня
				time(&temp);
				if (!(__global_fds->tlastp) ||
						(temp - __global_fds->tlastp) ||
						(__global_fds->sz[1] == __global_fds->sz[0]))
				{
					time(&(__global_fds->tlastp));
					fprintf (stdout, "\r %10.2f%% (%ld/%ld) %u",\
							((float)__global_fds->sz[1] / (float)__global_fds->sz[0]) * 100.0,\
							__global_fds->sz[1], __global_fds->sz[0], size);
					if (__global_fds->sz[1] == __global_fds->sz[0])
					{
						free (__global_fds);
						__global_fds = NULL;
						fprintf (stdout, "\n");
					}
					fflush (stdout);
				}
			}
		}
	}
	else
	{
		fprintf (stdout, " ERROR: loss write (): %p %p\n", true_write, write);
		fflush (stdout);
	}
	return ret;
}

ssize_t read (int fd, void *buf, size_t size)
{
	__lib_init ();
	ssize_t ret = 0;
	struct stat sstat;
	if (true_read)
	{
		// call to real
		ret = true_read (fd, buf, size);
		if (fd != STDIN_FILENO && __global_fds)
		{
			// init first
			// FIXME: very slow ._.
	//		if (__global_fds->fd[0] != fd)
	//		{
				__global_fds->fd[0] = fd;
				fstat (fd, &sstat);
				__global_fds->sz[0] = sstat.st_size;
	//		}
			// count
		}
	}
	else
	{
		fprintf (stdout, " ERROR: loss read (): %p %p\n", true_read, read);
		fflush (stdout);
	}
	return ret;
}

