/* vim: ft=c ff=unix fenc=utf-8
 * file: main.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

enum ping_version {
	PING_UNKNOWN,
	PING_IPUTILS,
	PING_BUSYBOX
};

void
read_ping_data(FILE *f)
{
	long transmitted = 0u;
	long received = 0u;
	float minimum = 0.f;
	float average = 0.f;
	float maximum = 0.f;

	char *line = NULL;
	size_t size = 0u;
	size_t len = 0u;

	enum ping_version pv = PING_UNKNOWN;
	int line_stage = 0;

	while(getline(&line, &size, f) != -1) {
		len = strlen(line);
		/* check header */
		if (len && pv == PING_UNKNOWN) {
			if (!strncmp(&line[len - 11], "data bytes\n", 11)) {
				/* busybox: "PING localhost (127.0.0.1): 56 data bytes\n" */
				pv = PING_BUSYBOX;
			} else if (!strncmp(&line[len - 15], "bytes of data.\n", 15)) {
				/* iputils: "PING localhost (127.0.0.1) 56(84) bytes of data.\n" */
				pv = PING_IPUTILS;
			} else {
				printf("Unknown ping version\n");
				break;
			}
		}

		if (line_stage == 1) {
			line_stage++;
			if (pv == PING_IPUTILS) {
				/* 2 packets transmitted, 2 received, 0% packet loss, time 999ms */
				sscanf(line,
						"%ld packets transmitted, %ld received",
						&transmitted, &received);
			} else if (pv == PING_BUSYBOX) {
				/* 2 packets transmitted, 2 packets received, 0% packet loss */
				sscanf(line,
						"%ld packets transmitted, %ld packets received",
						&transmitted, &received);
			}
		} else if (line_stage == 2) {
			if (pv == PING_IPUTILS) {
				/* rtt min/avg/max/mdev = 0.052/0.053/0.054/0.001 ms */
				sscanf(line, "rtt min/avg/max/mdev = %f/%f/%f",
						&minimum, &average, &maximum);
			} else if (pv == PING_BUSYBOX) {
				/* round-trip min/avg/max = 0.201/0.264/0.327 ms */
				sscanf(line,
						"round-trip min/avg/max = %f/%f/%f",
						&minimum, &average, &maximum);
			}
		} else if (line_stage == 0) {
			if (!strncmp("---", line, 3)) {
				line_stage++;
			}
		}
	}

	if (line)
		free(line);

	printf("ping data: min/avg/max: %f/%f/%f, transmitted/received: %ld/%ld\n",
			minimum, average, maximum, transmitted, received);
}

int
main(int argc, char *argv[])
{
	FILE *f = NULL;
	f = popen("ping -c 3 ya.ru", "r");
	read_ping_data(f);
	fclose(f);

	return EXIT_SUCCESS;
}

