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

#include "L2V.h"
#include "L2V_Funcs.h"
#include "L2V_Print.h"
#include <stdio.h>
#include <stdlib.h>

L2V_t L2V;


#define LAST_TREE_TEST 16
#define SIMPLE_SIZE (L2V_TREE_SIZE*LAST_TREE_TEST)


// Prototypes
static void RandomTest(void);
static void SequentialTest(void);
static void Simple_Init(void);
static void Simple_Update(UInt32 lba, UInt32 size, UInt32 vba);
static void Simple_Lookup(UInt32 lba, UInt32 *vba);

void Usage(int argc, char** argv)
{
    printf("Usage: %s s OR %s r (for sequential and random, respectively\n", argv[0], argv[0]);
    exit(-1);
}

int test = 0;

int main(int argc, char** argv)
{
    // we should add an option to change a pool size via command line 
    L2V_nodepool_mem = DEFAULT_L2V_NODEPOOL_MEM;

    //L2V_Init(SIMPLE_SIZE, 32768, 32768);
    //L2V_Init(SIMPLE_SIZE, 16384, 32768);
    //L2V_Init(SIMPLE_SIZE, 16384, 16384);
    //L2V_Init(SIMPLE_SIZE, 8192, 8192);
    L2V_Init(SIMPLE_SIZE, 8200, 8192);

    if (argc == 2) {
        if ('r' == argv[1][0]) {
            test = 0;
        } else if ('s' == argv[1][0]) {
            test = 1;
        } else {
            Usage(argc, argv);
        }
    }
    if (argc > 2) {
        Usage(argc, argv);
    }

    if (0 == test) {
        RandomTest();
    } else {
        SequentialTest();
    }

    return 0;
}


#define MAXLEN 512
static void RandomTest() {
    UInt32 lba, size, vba;
    UInt32 i, svba;
    L2V_SearchCtx_t c;

    L2V_Search_Init(&c);

    Simple_Init();

    sranddev();

    while (1) {
        lba = rand() % ((L2V_TREE_SIZE*LAST_TREE_TEST)-MAXLEN);
        size = (rand() % (MAXLEN-1))+1;
        vba = rand() % ((1UL<<(L2V.bits.vba-1))-size-1);

        printf("[UPD: lba:%d, size:%d, vba:%d]\n", lba, size, vba);
        L2V_Update(lba, size, vba, 0);
        Simple_Update(lba, size, vba);
        //printf("\n");
        //L2V_PrintNode(L2V_LPTR_TO_NODE(L2V.Tree[0]));
        L2V_PrintUsage(lba/L2V_TREE_SIZE);


        // Validate entire range
        int maxlevel=0;
        for (i = 0; i < SIMPLE_SIZE; i++) {
            Simple_Lookup(i, &svba);
            c.lba = i;
            L2V_Search(&c);
            if (c.level > maxlevel) maxlevel = c.level;
            if (svba != c.vba) {
                printf("\n\n\n");
                printf("====================\n");
                printf("MISMATCH lba:%d\n", i);
                printf("--------------------\n");
                printf("Simple vba:%d\n", svba);
                printf("Lookup vba:%d\n", c.vba);
            }
            l2v_assert_eq(svba, c.vba);
        }
        printf("maxlevel: %d\n", maxlevel);
        l2v_assert_lt(maxlevel, L2V_MAX_TREE_DEPTH);

#if 0
        if (repack++ > 10) {
            printf("Repacking tree %d...\n", nextRepack);
            L2V_Repack(nextRepack);
            L2V_PrintUsage(nextRepack);
            repack = 0;
            nextRepack = (nextRepack + 1) % LAST_TREE_TEST;
        }
#endif
    }
}

static void SequentialTest() {
    UInt32 lba, size, vba;
    UInt32 i, svba;
    L2V_SearchCtx_t c;

    L2V_Search_Init(&c);

    Simple_Init();

    sranddev();

    lba = 0;
    size = 0;
    vba = 1;

    while (1) {
        lba += size;
        size = (rand() % (MAXLEN-1))+1;
        if ((lba + size) >= L2V_TREE_SIZE) {
            lba = 0;
        }
        if ((rand() & 3) == 0) {
            vba = rand() % ((1UL<<(L2V.bits.vba-1))-size-1);
        }
        while (((vba + size) & ((1<<(L2V.bits.vba-1))-1)) < vba) {
            vba = rand() % ((1UL<<(L2V.bits.vba-1))-size-1);
        }

        printf("[UPD: lba:%d, size:%d, vba:%d]\n", lba, size, vba);
        L2V_Update(lba, size, vba, 0);
        Simple_Update(lba, size, vba);
        L2V_PrintUsage(lba/L2V_TREE_SIZE);
        vba += size;


        // Validate entire range
        int maxlevel=0;
        for (i = 0; i < SIMPLE_SIZE; i++) {
            Simple_Lookup(i, &svba);
            c.lba = i;
            L2V_Search(&c);
            if (c.level > maxlevel) maxlevel = c.level;
            if (svba != c.vba) {
                printf("\n\n\n");
                printf("====================\n");
                printf("MISMATCH lba:%d\n", i);
                printf("--------------------\n");
                printf("Simple vba:%d\n", svba);
                printf("Lookup vba:%d\n", c.vba);
            }
            l2v_assert_eq(svba, c.vba);
        }
        printf("maxlevel: %d\n", maxlevel);
        l2v_assert_lt(maxlevel, L2V_MAX_TREE_DEPTH);
    }
}


UInt32 Simple_vba[SIMPLE_SIZE];

static void Simple_Init()
{
    UInt32 i;

    for (i = 0; i < SIMPLE_SIZE; i++) {
        Simple_vba[i] = L2V_VBA_DEALLOC;
    }
}


static void Simple_Update(UInt32 lba, UInt32 size, UInt32 vba)
{
    while (size) {
        Simple_vba[lba] = vba;

        // Iterate
        lba++;
        if (vba < L2V_VBA_SPECIAL) {
            vba++;
        }
        size--;
    }
}


static void Simple_Lookup(UInt32 lba, UInt32 *vba)
{
    *vba = Simple_vba[lba];
}

void Outside_L2V_ValidUp(UInt32 vba, UInt32 count)
{
}
void Outside_L2V_ValidDown(UInt32 vba, UInt32 count)
{
}

