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
static void Simple_Update(UInt32 lba, UInt32 size, UInt32 vpn);
static void Simple_Lookup(UInt32 lba, UInt32 *vpn);

void Usage(int argc, char** argv)
{
    printf("Usage: %s s OR %s r (for sequential and random, respectively\n", argv[0], argv[0]);
    exit(-1);
}

int test = 0;

int main(int argc, char** argv)
{
    L2V_Init(SIMPLE_SIZE, (L2V_VPN_SPECIAL-1) / 8192, 8192);

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
    UInt32 lba, size, vpn;
    UInt32 i, svpn;
    L2V_SearchCtx_t c;

    L2V_Search_Init(&c);

    Simple_Init();

    sranddev();

    while (1) {
        lba = rand() % ((L2V_TREE_SIZE*LAST_TREE_TEST)-MAXLEN);
        size = (rand() % (MAXLEN-1))+1;
        vpn = rand() % ((1<<(L2V_BITS_VPN-1))-size-1);

        printf("[UPD: lba:%d, size:%d, vpn:%d]\n", lba, size, vpn);
        L2V_Update(lba, size, vpn);
        Simple_Update(lba, size, vpn);
        //printf("\n");
        //L2V_PrintNode(L2V_LPTR_TO_NODE(L2V.Tree[0]));
        L2V_PrintUsage(0);


        // Validate entire range
        int maxlevel=0;
        for (i = 0; i < SIMPLE_SIZE; i++) {
            Simple_Lookup(i, &svpn);
            c.lba = i;
            L2V_Search(&c);
            if (c.level > maxlevel) maxlevel = c.level;
            if (svpn != c.vpn) {
                printf("\n\n\n");
                printf("====================\n");
                printf("MISMATCH lba:%d\n", i);
                printf("--------------------\n");
                printf("Simple vpn:%d\n", svpn);
                printf("Lookup vpn:%d\n", c.vpn);
            }
            l2v_assert_eq(svpn, c.vpn);
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
    UInt32 lba, size, vpn;
    UInt32 i, svpn;
    L2V_SearchCtx_t c;

    L2V_Search_Init(&c);

    Simple_Init();

    sranddev();

    lba = 0;
    size = 0;
    vpn = 1;

    while (1) {
        lba += size;
        size = (rand() % (MAXLEN-1))+1;
        if ((lba + size) >= L2V_TREE_SIZE) {
            lba = 0;
        }
        if ((rand() & 3) == 0) {
            vpn = rand() % ((1<<(L2V_BITS_VPN-1))-size-1);
        }
        while (((vpn + size) & ((1<<(L2V_BITS_VPN-1))-1)) < vpn) {
            vpn = rand() % ((1<<(L2V_BITS_VPN-1))-size-1);
        }

        printf("[UPD: lba:%d, size:%d, vpn:%d]\n", lba, size, vpn);
        L2V_Update(lba, size, vpn);
        Simple_Update(lba, size, vpn);
        L2V_PrintUsage(0);
        vpn += size;


        // Validate entire range
        int maxlevel=0;
        for (i = 0; i < SIMPLE_SIZE; i++) {
            Simple_Lookup(i, &svpn);
            c.lba = i;
            L2V_Search(&c);
            if (c.level > maxlevel) maxlevel = c.level;
            if (svpn != c.vpn) {
                printf("\n\n\n");
                printf("====================\n");
                printf("MISMATCH lba:%d\n", i);
                printf("--------------------\n");
                printf("Simple vpn:%d\n", svpn);
                printf("Lookup vpn:%d\n", c.vpn);
            }
            l2v_assert_eq(svpn, c.vpn);
        }
        printf("maxlevel: %d\n", maxlevel);
        l2v_assert_lt(maxlevel, L2V_MAX_TREE_DEPTH);
    }
}


UInt32 Simple_vpn[SIMPLE_SIZE];

static void Simple_Init()
{
    UInt32 i;

    for (i = 0; i < SIMPLE_SIZE; i++) {
        Simple_vpn[i] = L2V_VPN_MISS;
    }
}


static void Simple_Update(UInt32 lba, UInt32 size, UInt32 vpn)
{
    while (size) {
        Simple_vpn[lba] = vpn;

        // Iterate
        lba++;
        if (vpn < L2V_VPN_SPECIAL) {
            vpn++;
        }
        size--;
    }
}


static void Simple_Lookup(UInt32 lba, UInt32 *vpn)
{
    *vpn = Simple_vpn[lba];
}

