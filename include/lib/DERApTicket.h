/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */
#ifndef DERAPTICKET_H
#define DERAPTICKET_H

#include <stdbool.h>
#include <libDER/libDER.h>

/* AP Ticket */
typedef struct {
    DERItem         signatureAlgorithm;     /* AlgorithmId */
    DERItem         body;                   /* SET OF OCTECT STRING, DER_DEC_SAVE_DER */
    DERItem         signature;              /* OCTET STRING */
    DERItem         certificates;           /* SEQUENCE of CERTIFICATE */
} DERApTicket;

/* Cert extension */
typedef struct {
    DERItem         version;                /* current version is 0; omitted until version 1 */
    DERItem         override;
    DERItem         append;                 /* optional; unsupported in version 0 */
    DERItem         remove;                 /* optional; unsupported in version 0 */
} DERApTicketCertExtension;

/* DERItemSpecs to decode into a DERApTicket */
extern const DERItemSpec DERApTicketItemSpecs[];
extern const DERSize DERNumApTicketItemSpecs;

DERReturn
DERApTicketCopyTagData( 
    const DERItem *container,
    DERTag tag, 
    uint8_t *buffer,
    DERSize *ioLength );
    
DERReturn 
DERApTicketDecode(
    const DERItem * contents,
    DERApTicket *   outTicket,
    DERSize *       outNumUsedBytes );

DERReturn
DERApTicketValidate( 
    const DERApTicket *ticket,
    bool isRestore );

DERReturn
DERApTicketParseLengthFromBuffer( 
    const uint8_t * buffer, 
    size_t          length, 
    size_t *        outTicketLength );

#endif /* DERAPTICKET_H */
