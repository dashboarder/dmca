#ifndef __OSBYTEORDER_H_
#define __OSBYTEORDER_H_

/* This is a horrible hack; remove this header as soon as libcorecrypto stops including it.
 */

#define OSSwapHostToBigInt32(x) ((uint32_t)(x))
#define OSSwapHostToBigInt(x)   OSSwapHostToBigInt32(x)

#endif /* __OSBYTEORDER_H_ */
