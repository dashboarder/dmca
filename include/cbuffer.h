/*
 * Copyright (C) 2006 Apple Computer, Inc. All rights reserved.
 *
 * This document is the property of Apple Computer, Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Computer, Inc.
 */
#ifndef _CBUFER_H_
#define _CBUFER_H_

#include <sys/types.h>
#include <sys.h>
#include <lib/libc.h>

__BEGIN_DECLS

#define CB_ALIGNMENT      64

typedef struct _CBUFFER {
  uint8_t   * head;
  uint8_t   * tail;
  size_t   size;
  size_t   readableSize;
  size_t    bufferSize;
  uint8_t   * dataStart;
  uint8_t   * dataEnd;
} CBUFFER;

#define cb_fix_high( ptr, pcb ) \
  if( pcb->ptr >= pcb->dataEnd ) { \
    pcb->ptr = pcb->dataStart; \
  }

#define cb_cs_encap( theCall, retType ) \
  retType ret; \
  enter_critical_section(); \
  ret = theCall; \
  exit_critical_section(); \
  return ret



static inline bool cb_initialized( CBUFFER * pcb )
{
  return pcb->dataStart?true:false;
}

static inline int cb_create( CBUFFER * pcb, size_t size )
{
  // round up
  size = (size + CB_ALIGNMENT-1) & ~(CB_ALIGNMENT-1);
  pcb->dataStart = (uint8_t *)memalign( size, CB_ALIGNMENT );

  pcb->head = pcb->tail = pcb->dataStart;
  pcb->dataEnd = pcb->dataStart+size;
  pcb->size = 0;
  pcb->readableSize = 0;
  pcb->bufferSize = size;

  return size;
}

static inline void cb_reset( CBUFFER * pcb )
{
  enter_critical_section();
  pcb->head = pcb->tail = pcb->dataStart;
  pcb->size = 0;
  pcb->readableSize = 0;
  pcb->bufferSize = pcb->dataEnd - pcb->dataStart;
  exit_critical_section();
}

static inline uint32_t cb_size( CBUFFER * pcb) 
{
  return pcb->size;
}

static inline uint32_t cb_readable_size( CBUFFER * pcb )
{
  return pcb->readableSize;
}

static inline uint8_t * cb_get_head( CBUFFER * pcb )
{
  return pcb->head;
}

static inline uint32_t cb_free_space( CBUFFER * pcb )
{
  return pcb->bufferSize - cb_size(pcb);
}

static inline uint32_t cb_block_free_space_unsafe( CBUFFER * pcb )
{
  uint32_t freeTotal = cb_free_space(pcb);
  uint32_t freeTillEnd = pcb->dataEnd-pcb->head;

  return __min( freeTotal, freeTillEnd );
}


static inline uint32_t cb_block_free_space( CBUFFER * pcb )
{
  cb_cs_encap( cb_block_free_space_unsafe( pcb ), uint32_t );
}

static inline uint8_t *  cb_block_read_ptr_unsafe( CBUFFER * pcb, uint32_t * pamount )
{
  uint8_t * ret = pcb->tail;

  /*printf( "data 0x%08X, head 0x%08X, tail 0x%08X, data end %08X, size %d\n",
    (uint)pcb->dataStart, (uint)pcb->head, (uint)pcb->tail, (uint)pcb->dataEnd, (uint)cb_size( pcb ) );  */
  if(  pcb->head <= pcb->tail ){
    // header is past tail, so there is a wrap around
    *pamount = pcb->dataEnd-pcb->tail;
    pcb->size -= *pamount;
    pcb->readableSize -= *pamount;
    pcb->tail = pcb->dataStart;
  } else {
    // we can consume all of it
    pcb->tail = pcb->head;
    *pamount = pcb->size;
    pcb->size = 0;
    pcb->readableSize = 0;
  }
  /*printf( "- data 0x%08X, head 0x%08X, tail 0x%08X, data end %08X, size %d\n",
    (uint)pcb->dataStart, (uint)pcb->head, (uint)pcb->tail, (uint)pcb->dataEnd, (uint)cb_size( pcb ) );  */
  return ret;
}

static inline uint8_t *  cb_block_read_ptr( CBUFFER * pcb, uint32_t * pamount )
{
  cb_cs_encap( cb_block_read_ptr_unsafe( pcb, pamount ), uint8_t * );
}

static inline void cb_pre_skip( CBUFFER * pcb, uint32_t amount )
{
  uint32_t bfs;
  enter_critical_section();
  
  pcb->size += amount;

  bfs = pcb->dataEnd-pcb->head;

  if( bfs > amount ){
    //printf( "a bfs %d, amount %d\n", bfs, amount );
    pcb->head += amount;
  } else {
    //printf( "b bfs %d, amount %d\n", bfs, amount );
    pcb->head = pcb->dataStart + amount-bfs;
  }

  exit_critical_section();

}

static inline void cb_post_skip( CBUFFER * pcb, uint32_t amount )
{

  enter_critical_section();


  /*printf( "$ data 0x%08X, head 0x%08X, tail 0x%08X, data end %08X, size %d\n",
    (uint)pcb->dataStart, (uint)pcb->head, (uint)pcb->tail, (uint)pcb->dataEnd, (uint)cb_size( pcb ) );  */

  // update internal count to reflect the new data now available to read
  pcb->readableSize += amount;


  /*printf( "* data 0x%08X, head 0x%08X, tail 0x%08X, data end %08X, size %d\n",
    (uint)pcb->dataStart, (uint)pcb->head, (uint)pcb->tail, (uint)pcb->dataEnd, (uint)cb_size( pcb ) );  */

  exit_critical_section();

}

static inline int cb_putc_unsafe( CBUFFER * pcb, uint8_t c )
{
  if( cb_free_space( pcb ) ){
    *pcb->head++ = c;

    cb_fix_high( head, pcb );

    pcb->size++;
    pcb->readableSize++;
    return c;
  } else {
    return -1;
  }
}


static inline int cb_putc( CBUFFER * pcb, uint8_t c )
{
  cb_cs_encap( cb_putc_unsafe( pcb, c ), int );
}

static inline int cb_peekc_unsafe( CBUFFER * pcb )
{
  if( cb_size( pcb ) ){
    return *pcb->tail;
  } else {
    return -1;
  }
}


static inline int cb_peekc( CBUFFER * pcb )
{
  cb_cs_encap( cb_peekc_unsafe( pcb ), int );
}

static inline int cb_getc_unsafe( CBUFFER * pcb  )
{
  if( cb_size( pcb ) ){
    uint8_t ret = *pcb->tail++;
    
    cb_fix_high( tail, pcb );
    
    pcb->size--;
    pcb->readableSize--;

    return ret;
  } else {
    return -1;
  }
}

static inline int cb_getc( CBUFFER * pcb  )
{
  cb_cs_encap( cb_getc_unsafe( pcb ), int );
}
    
static inline uint32_t cb_read_unsafe( CBUFFER * pcb, uint8_t * buffer, size_t size )
{
  uint32_t readable = cb_readable_size( pcb );
  if( readable ){
    size = __min( size, readable );
    size_t blockSize = pcb->dataEnd-pcb->tail;

    if( blockSize < size ){
      // we must copy some part of the data from tail-->dataEnd
      // and then copy the rest from dataStart
      memcpy( buffer, pcb->tail, blockSize );
      buffer += blockSize;

      blockSize = size - blockSize;
      pcb->tail = pcb->dataStart + blockSize;
      memcpy( buffer, pcb->dataStart, blockSize );

    } else {
      // we can copy the entire block from tail 
      memcpy( buffer, pcb->tail, size );

      pcb->tail += size;

    }

    pcb->size -= size;
    pcb->readableSize -= size;

    return size;
  } else {
    return 0;
  }
}

static inline uint32_t cb_read( CBUFFER * pcb, uint8_t * buffer, size_t size )
{
  cb_cs_encap( cb_read_unsafe( pcb, buffer, size ), uint32_t );
}

static inline int cb_write_unsafe( CBUFFER * pcb, const uint8_t * buffer, size_t size )
{
  if( cb_free_space( pcb ) ){
    size = __min( size, pcb->bufferSize-pcb->size );
    size_t blockSize = pcb->dataEnd-pcb->head;
    if( blockSize < size ){
      // must write in blocks from head-->dataEnd
      // and then from dataStart
      memcpy( pcb->head, buffer, blockSize );
      buffer += blockSize;

      blockSize = size - blockSize;
      pcb->head = pcb->dataStart + blockSize;
      memcpy( pcb->dataStart, buffer, blockSize );

    } else {
      memcpy( pcb->head, buffer, size );
      pcb->head += size;
      
    }
    
    pcb->size += size;
    pcb->readableSize += size;

    return size;
  } else {
    return 0;
  }
}


static inline int cb_write( CBUFFER * pcb, const uint8_t * buffer, size_t size )
{
  cb_cs_encap( cb_write_unsafe( pcb, buffer, size ), int );
}

static inline void cb_free( CBUFFER * pcb )
{
  free( pcb->dataStart );
  pcb->dataEnd = pcb->dataStart = NULL;
}

__END_DECLS

#endif
