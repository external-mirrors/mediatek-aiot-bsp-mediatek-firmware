// SPDX-License-Identifier: MIT
/*
 * MediaTek Firmware extractor
 *
 * This tool allows extracting the actual binary firmware
 * from firmware partition images used in SPM Yocto, created
 * by analyzing the corresponding binary files' format.
 *
 * Build: gcc -Wall -Werror -o extract-mtk extract-mtk.c
 * Usage: ./extract-mtk <firmware partition image>
 *
 * Copyright (C) 2026 Collabora Ltd
 *	Arnaud Ferraris <arnaud.ferraris@collabora.com>
 */

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/stat.h>

typedef struct {
	uint32_t size_magic;
	uint32_t size;
	char entry_name[40];
	uint32_t offset_magic;
	uint32_t offset;
} hdr_t;

int main (int argc, char *argv[])
{
	int fd, ret;
	hdr_t header;
	off_t base_pos = 0;
	struct stat file_stat;

	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		printf("Unable to open %s\n", argv[1]);
		return -1;
	}
	fstat(fd, &file_stat);

	do {
		off_t padding;
		uint8_t *buf;
		char dest_name[128];
		int dest;

		ret = read(fd, &header, sizeof(header));
		if (ret < 0) {
			printf("Unable to read header @ 0x%08lx\n", base_pos);
			return -1;
		}

		if (header.size_magic != 0x58881688) {
			printf("Wrong value for size magic: 0x%08x\n", header.size_magic);
			return -1;
		}

		if (header.offset_magic != 0x58891689) {
			printf("Wrong value for offset magic: 0x%08x\n", header.size_magic);
			return -1;
		}

		printf("Will read 0x%08x bytes @ 0x%08lx\n", header.size, base_pos + header.offset);
		buf = malloc(header.size);
		if (!buf) {
			printf("Unable to allocate %u bytes\n", header.size);
			return -1;
		}

		base_pos = lseek(fd, base_pos + header.offset, SEEK_SET);
		ret = read(fd, buf, header.size);
		if (ret < 0) {
			printf("Unable to read data @ 0x%08lx\n", base_pos);
			return -1;
		}

		sprintf(dest_name, "%s.%s", argv[1], header.entry_name);
		unlink(dest_name);
		dest = open(dest_name, O_CREAT | O_WRONLY | O_TRUNC, 0666);
		if (dest < 0) {
			printf("Unable to create %s\n", dest_name);
			return -1;
		}

		printf("Will write 0x%08x bytes to %s\n", header.size, dest_name);
		ret = write(dest, buf, header.size);
		if (ret < 0) {
			printf("Unable to write data\n");
			return -1;
		}

		close(dest);

		padding = header.size % 16;
		if (padding) {
			padding = 16 - padding;
		}
		base_pos = lseek(fd, padding, SEEK_CUR);

		free(buf);
	} while (base_pos > 0 && base_pos < file_stat.st_size);

	close(fd);

	return 0;
}
