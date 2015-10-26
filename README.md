# Memory management (v1.1.0)
Memory management in C

## Global typedef

`CMM_free_func` is defined as a type of function pointer taking a void pointer
and return void. It is used as a type for `free()` function.

`CMM_handle` is defined as a type for memory management handle.

## Global interfaces (recommended)
 
### `int CMM_Init(size_t buckets)`

This is the initialization function that should be called before using the
memory management. The parameter specifies the number of buckets used for hash
table.  More buckets can decrease the probability of conflict but incur more
memory overhead. In the case of allocation failure, the function returns 1.

### `void CMM_Finalize()`

This is the finalization function that clean up the memory that managed by the
handle and the handle itself.

### `void* CMM_Malloc(size_t)`

Basically, this function calls `malloc` for the memory and register the memory
in the management unit. In case the `malloc` returns NULL, this function returns
NULL. This function is recommended for basic use.

### `int CMM_Register_memory(void*)`

Manually register the memory to the management unit. The return codes and their
meaning are as follows:

* 0 - successful
* 1 - failed because insufficient memory
* 2 = called before `CMM_Init` or after `CMM_Finalize`

### `int CMM_Register_special_memory(void*, CMM_free_func)`

Manually register the memory to the management unit. The only difference between
this function and the previous function is that custom free function can be
specified to free this chunk memory. The return code is the same as previous
function.

### `void CMM_Set_free_func(CMM_free_func)`

This function set the default free function.

### `int CMM_Free(void*)`

This function free the specified memory in the management unit. The return codes
and their meaning are as follows:

* 0 - successful
* 1 - the memory pointed by the pointer is not found in the management unit
* 2 - pointer is `NULL`
* 3 - called before `CMM_Init` or after `CMM_Finalize`

## Local interfaces (expert interfaces)

### `CMM_handle* CMM_init(size_t buckets)`

This is the initialization function that should be called before using the
memory management. The parameter specifies the number of buckets used for hash
table.  More buckets can decrease the probability of conflict but incur more
memory overhead. The return handle should be check against NULL. In the case of
allocation failure, the function returns NULL pointer.

### `void CMM_finalize(CMM_handle*)`

This is the finalization function that clean up the memory that managed by the
handle and the handle itself. This function must be call before the handle
variable becomes out of scope otherwise memory leak is ensure (even user
manually free the memory, the internal memory in the memory management unit will
certainly leak).

### `void* CMM_malloc(CMM_handle*, size_t)`

Basically, this function calls `malloc` for the memory and register the memory
in the management unit. In case the `malloc` returns NULL, this function returns
NULL. This function is recommended for basic use.

### `int CMM_register_memory(CMM_handle*, void*)`

Manually register the memory to the management unit. The return codes and their
meaning are as follows:

* 0 - successful
* 1 - failed because insufficient memory
* 2 = `CMM_handle` is `NULL`

### `int CMM_register_special_memory(CMM_handle*, void*, CMM_free_func)`

Manually register the memory to the management unit. The only difference between
this function and the previous function is that custom free function can be
specified to free this chunk memory. The return code is the same as previous
function.

### `void CMM_set_free_func(CMM_handle*, CMM_free_func)`

This function set the default free function.

### `int CMM_free(CMM_handle*, void*)`

This function free the specified memory in the management unit. The return codes
and their meaning are as follows:

* 0 - successful
* 1 - the memory pointed by the pointer is not found in the management unit
* 2 - pointer is `NULL`
* 3 - `CMM_handle` is `NULL`

## Usage tips (expert interfaces)

* Initialize a management unit at the beginning of the function.
* Use `CMM_malloc` to ease the way to get and register the memory.
* Call `CMM_finalize` at the end of the function to clean up. 
* `CMM_free` is totally optional.
