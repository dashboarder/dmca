#ifndef _WMR_TYPES_H_
#define _WMR_TYPES_H_

# include <sys/types.h>
# include "spTypes.h"

/*****************************************************************************/
/* Basic Types                                                               */
/*****************************************************************************/

typedef     UInt32 BOOL32;

/*****************************************************************************/
/* Basic Constants                                                           */
/*****************************************************************************/

#define     FALSE32                 ((BOOL32)0)
#define     TRUE32                  ((BOOL32)1)

#ifndef     NULL
#ifdef      __cplusplus
#define     NULL                0
#else
#define     NULL                ((void *)0)
#endif
#endif

# define __align(_x)
/*****************************************************************************/
/* Basic Compiler Flags                                                      */
/*****************************************************************************/

#define INLINE  __inline

#ifdef DEBUG_BUILD
#define WMR_DEBUG (1)
#else
#define WMR_RELEASE (1)
#endif

#endif /* _WMR_TYPES_H_ */

