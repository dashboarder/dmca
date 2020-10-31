/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <mach-o/loader.h>
#include <mach/arm/thread_status.h>
#include <sys/types.h>

struct task {
    uint32_t	cmd;		/* LC_THREAD or  LC_UNIXTHREAD */
    uint32_t	cmdsize;	/* total size of this command */
    uint32_t flavor;		/*   flavor of thread state */
    uint32_t count;		  /* count of longs in thread state */
    arm_thread_state_t arm_state; /*   thread state for this flavor */
};


int main (int argc, const char *argv[])
{
    FILE *out;
    FILE *in;
    DIR  *dir;
    const char* src_dir;
    int num_segments;
    u_int32_t data_cursor;
    struct dirent *ep;
    struct mach_header header = 
    {
        MH_MAGIC,
        CPU_TYPE_ARM,
        CPU_SUBTYPE_ARM_V4T,
        MH_CORE
    };
    struct task thread = {0};

    if (argc != 3)
    {
        printf("%s <iop.memory dir> <output corefile>\n", argv[0]);
        return -1;
    }

    // Open directory
    src_dir = argv[1];
    dir = opendir(src_dir);
    if (!dir)
        return -2;

    // Open output core file
    out = fopen(argv[2], "wb");
    if (!out)
        return -3;
    
    // Lay down a heder (to be updated later)
    fwrite(&header, 1, sizeof(header), out);
    
    // Lay down some thread context
    thread.cmd = LC_THREAD;
    thread.cmdsize = sizeof(thread);
    
    thread.flavor = ARM_THREAD_STATE;
    thread.count = ARM_THREAD_STATE_COUNT;
    
    // XXX fill in registers
    // thread.arm_state.r[13] =
    // thread.arm_state.sp =
    // thread.arm_state.lr =
    // thread.arm_state.pc =
    // thread.arm_state.cpsr =
    
    fwrite(&thread, 1, sizeof(thread), out);
    
    header.ncmds += 1;
    header.sizeofcmds += sizeof(thread);
    
    // To make life easier below, count the number of segments we're going to have
    num_segments = 0;
    while(ep = readdir(dir))
    {
        int addr;
        if (1 == sscanf(ep->d_name, "%x.bin", &addr))
            num_segments += 1;
    }
    // Rewind to actually process these files now 
    // XXX assume contents will not change from the scan above
    closedir(dir);
    dir = opendir(src_dir);
    
    // For each file, create a segment based on its filename
    chdir(src_dir);
    data_cursor = sizeof(header) + 
                  header.sizeofcmds + 
                  (num_segments * sizeof(struct segment_command));
    while(ep = readdir(dir))
    {
        struct segment_command sc = {0};
        
        // Filenames are <address>.bin
        if (1 == sscanf(ep->d_name, "%x.bin", &sc.vmaddr))
        {
            // Create a segment with the given address and file contents
            in = fopen(ep->d_name, "rb");
            if (in)
            {
                void * data;
                int old_pos;
                
                // Record how big the source file is
                fseek(in, 0, SEEK_END);
                sc.vmsize = ftell(in);
                fseek(in, 0, SEEK_SET);
                
                // Set up a zero-section text segment load command
                sc.cmd = LC_SEGMENT;
                sc.cmdsize = sizeof(sc);
                strcpy(sc.segname, SEG_TEXT);
                
                // Populate the entire segment, no gaps
                // Use the data cursor pointing past the end of the load commands
                sc.filesize = sc.vmsize;
                sc.fileoff = data_cursor;
                data_cursor += sc.filesize;
                
                // Write out the load command
                fwrite(&sc, 1, sizeof(sc), out);
                
                
                // Read out the data
                data = malloc(sc.vmsize);
                if (!data)
                    return -4;
                fread(data, 1, sc.vmsize, in);
                fclose(in);
                
                // Put the data at the current cursor command
                old_pos = ftell(out);
                if (fseek(out, sc.fileoff, SEEK_SET))
                    return -5;
                fwrite(data, 1, sc.vmsize, out);
                free(data);
                
                // Put the cursor back for the next load command
                fseek(out, old_pos, SEEK_SET);
                
                // Increment the number of load commands in the mach header
                header.ncmds += 1;
                header.sizeofcmds += sizeof(sc);
            }
            else
            {
                fprintf(stderr, "can't open %s\n", ep->d_name);
            }
        }
    }
    closedir(dir);
    
    // Update the header with how many segments we actually generated
    fseek(out, 0, SEEK_SET);
    fwrite(&header, 1, sizeof(header), out);
    fclose(out);
    

    return 0;
}
