/*
 *  Copyright (C) 2009  Apple, Inc. All rights reserved.
 *  
 *  This document is the property of Apple Inc.
 *  It is considered confidential and proprietary.
 *
 *  This document may not be reproduced or transmitted in any form,
 *  in whole or in part, without the express written permission of
 *  Apple Inc.
 */
/*
 * Extract the interesting parts of a Mach-O binary into a preloaded memory image.
 *
 * We assume that we want all of the __TEXT segment, and anything in __DATA that
 * isn't zero-filled.  
 *
 * We expect that the linker will have laid out the data in this object file with
 * sections appropriately sized and aligned for directly transferring to memory.
 * This is normally the case for MH_PRELOAD objects.
 *
 * Currently only supports 32-bit Mach-O.
 */

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <mach-o/loader.h>

void            output_sections(int fd, unsigned char *buf, struct section *sect, uint32_t nsects);
void            usage(void);

int
main(int argc, char *argv[])
{
        int                     src, dst;
        unsigned char           *buf;
        struct mach_header      *mh;
        struct stat             s;
        unsigned char           *cursor;
        uint32_t                cmdcount;
        struct load_command     *cmd;
        struct segment_command  *seg;

        /* check args, open files */
        if (3 != argc)
                usage();
        if ((src = open(argv[1], O_RDONLY)) < 0)
                err(1, "can't open '%s'", argv[1]);
        if (!strcmp(argv[2], "-")) {
                dst = STDOUT_FILENO;
        } else {
                if ((dst = open(argv[2], O_CREAT | O_TRUNC | O_WRONLY, 0600)) < 0)
                        err(1, "can't open '%s'", argv[2]);
        }

        /* read the source file */
        if (fstat(src, &s) < 0)
                err(1, "can't stat '%s'", argv[1]);
        if (NULL == (buf = (unsigned char *)malloc(s.st_size)))
                errx(1, "can't allocate %llu bytes for input file", s.st_size);
        if (read(src, buf, s.st_size) != s.st_size)
                err(1, "can't read %llu bytes from input file", s.st_size);
        close(src);

        /* do some quick sanity on the input */
        mh = (struct mach_header *)buf;
        if (mh->magic != MH_MAGIC)
                errx(1, "unsupported file type (magic 0x%08x)", mh->magic);
        if (mh->filetype != MH_PRELOAD)
                errx(1, "unsupported file type (filetype 0x%08x)", mh->filetype);
        if (!(mh->flags & MH_NOUNDEFS))
                errx(1, "file has undefined symbols");

        /* process load commands */
        cursor = (unsigned char *)(mh + 1);
        cmdcount = mh->ncmds;

        while (cmdcount-- > 0) {
                cmd = (struct load_command *)cursor;

                /* is this a segment command? */
                if (cmd->cmd == LC_SEGMENT) {
                        seg = (struct segment_command *)cmd;
                        if (!strcmp("__TEXT", seg->segname)) {
                                /* emit all of the sections in this segment, back to back */
                                output_sections(dst, buf, (struct section *)(seg + 1), seg->nsects);
                                
                        } 
                        if (!strcmp("__DATA", seg->segname)) {
                                /* emit sections in this segment, stopping if/when we hit a zero-fill section */
                                output_sections(dst, buf, (struct section *)(seg + 1), seg->nsects);
                        }
                }
                cursor += cmd->cmdsize;
        }
        free(buf);
        if (STDOUT_FILENO != dst)
                close(dst);
        return(0);
}

void
output_sections(int fd, unsigned char *buf, struct section *sect, uint32_t nsects)
{
        
        while (nsects-- > 0) {
                /* some linkers forget to set the S_ZEROFILL flag on zerofill sections */
                if ((sect->flags & S_ZEROFILL) ||
                    !strcmp(sect->sectname, "__common") ||
                    !strcmp(sect->sectname, "__bss"))
                        return;
                
                //warnx("writing %s,%s", sect->segname, sect->sectname);
                        
                /* write section data from memory to output */
                if (write(fd, buf + sect->offset, sect->size) != sect->size)
                        err(1, "error writing to output file");
                sect++;
        }
}

void
usage(void)
{
        warnx("usage:");
        fprintf(stderr, "\n  %s <mach-o file> {<binary file>|-}\n\n", getprogname());
        exit(1);
}

