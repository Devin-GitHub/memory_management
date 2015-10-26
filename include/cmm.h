#ifndef CMM_CMM_H
#define CMM_CMM_H

#include <stddef.h>

typedef void (*CMM_free_func)(void*);

// main handle for memory management.
typedef struct CMM_handle CMM_handle;

// get version
long cmm_version();

/************************************************************************
 *
 * Recommended interfaces.
 *
 ***********************************************************************/

/* initialization
 * return code:
 *   0 - successful
 *   1 - failed because of insuficient memory
 */
int CMM_Init(size_t buckets);

// finalization
void CMM_Finalize();

// get memory
void* CMM_Malloc(size_t);
/* register otherwise allocated memory
 * return code:
 *   0 - successful
 *   1 - failed because of insufficient memory
 *   2 - called before CMM_Init or after CMM_Finalize
 */
int CMM_Register_memory(void*);
int CMM_Register_special_memory(void*, CMM_free_func);
// set default free function
void CMM_Set_free_func(CMM_free_func);
/* free memory
 * return code:
 *   0 - successful
 *   1 - memory not found
 *   2 - cannot free NULL
 *   3 - called before CMM_Init or after CMM_Finalize
 */
int CMM_Free(void*);

/************************************************************************
 *
 * Expert interfaces.
 *
 ***********************************************************************/

// initialization
// return NULL if insufficient memory
CMM_handle* CMM_init(size_t buckets);
// finalization
void CMM_finalize(CMM_handle*);

// get memory
void* CMM_malloc(CMM_handle*, size_t);
/* register otherwise allocated memory
 * return code:
 *   0 - successful
 *   1 - failed because of insufficient memory
 *   2 - CMM_handle is NULL
 */
int CMM_register_memory(CMM_handle*, void*);
int CMM_register_special_memory(CMM_handle*, void*, CMM_free_func);
// set default free function
void CMM_set_free_func(CMM_handle*, CMM_free_func);
/* free memory
 * return code:
 *   0 - successful
 *   1 - memory not found
 *   2 - cannot free NULL
 *   3 - CMM_handle is NULL
 */
int CMM_free(CMM_handle*, void*);

#endif
