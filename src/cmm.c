#include "cmm.h"
#include <stdlib.h>
#include <math.h>

typedef struct Node Node;
void free_node(CMM_handle* cmm_handle, Node* node_ptr);
void free_hash_table(CMM_handle* cmm_handle);
void recycle_node(CMM_handle* cmm_handle, Node* node_ptr);
void free_mem_chain(CMM_handle* cmm_handle);
Node* make_chunk_nodes(size_t counts);
int get_one_free_node(CMM_handle* cmm_handle, Node** new_node);
int register_memory(CMM_handle* cmm_handle, void* data,
        CMM_free_func free_func);
size_t simple_hash(CMM_handle* cmm_handle, void* data);

struct Node
{
    long long id;
    void *data;
    CMM_free_func free_func;
    Node *next;
};

struct CMM_handle
{
    size_t buckets;
    size_t hash_factor;
    size_t (*hash_func)(CMM_handle*, void*);
    Node *mem, *mem_tail;
    Node *free_mem_stack;
    Node **hash_table;
    CMM_free_func free_func;
};

CMM_handle* CMM_init(size_t buckets)
{
    CMM_handle *cmm_handle = malloc(sizeof(CMM_handle));
    if (cmm_handle == NULL)
        return NULL;
    cmm_handle->buckets = buckets;
    cmm_handle->hash_factor = sizeof(long double);
    cmm_handle->hash_func = simple_hash;
    cmm_handle->mem = cmm_handle->mem_tail = make_chunk_nodes(buckets / 10);
    if (cmm_handle->mem == NULL)
        return NULL;
    cmm_handle->free_mem_stack = cmm_handle->mem->data;
    cmm_handle->hash_table = malloc(buckets * sizeof(CMM_handle*));
    size_t i = 0;
    for (; i < buckets; ++i)
        cmm_handle->hash_table[i] = NULL;
    cmm_handle->free_func = free;
    return cmm_handle;
}

void CMM_finalize(CMM_handle* cmm_handle)
{
    free_hash_table(cmm_handle);
    free_mem_chain(cmm_handle);
    free(cmm_handle);
}

void* CMM_malloc(CMM_handle *cmm_handle, size_t size)
{
    void *data = malloc(size);
    if (data == NULL)
        return NULL;
    if (CMM_register_memory(cmm_handle, data))
        return NULL;
    return data;
}

int CMM_register_memory(CMM_handle *cmm_handle, void *data)
{
    return register_memory(cmm_handle, data, NULL);
}

int CMM_register_special_memory(CMM_handle *cmm_handle, void *data,
        CMM_free_func free_func)
{
    return register_memory(cmm_handle, data, free_func);
}

void CMM_set_free_func(CMM_handle* cmm_handle, CMM_free_func free_func)
{
    cmm_handle->free_func = free_func;
}

int CMM_free(CMM_handle *cmm_handle, void *data)
{
    if (data == NULL)
        return 2;
    const size_t index = cmm_handle->hash_func(cmm_handle, data);
    Node *scanner = cmm_handle->hash_table[index], *prev = NULL;
    while (scanner != NULL && scanner->data != data)
    {
        prev = scanner;
        scanner = scanner->next;
    }
    if (scanner == NULL)
        return 1;
    if (prev == NULL)
        cmm_handle->hash_table[index] = scanner->next;
    else
        prev->next = scanner->next;
    free_node(cmm_handle, scanner);
    recycle_node(cmm_handle, scanner);
    return 0;
}


/* -------------------------------------------------------------------------- 
 * private functions
 * -------------------------------------------------------------------------- */

Node* make_chunk_nodes(size_t counts)
{
    if (counts == 0) counts = 1;
    static long long node_id = 0, handle_node_id = -1;
    if (node_id != 0)
        counts = (size_t)llabs(node_id);
    Node *raw_ptr = malloc((counts + 1) * sizeof(Node));
    if (raw_ptr == NULL)
        return NULL;
    raw_ptr->id = handle_node_id;
    raw_ptr->data = raw_ptr + 1;
    raw_ptr->free_func = NULL;
    raw_ptr->next = NULL;
    Node *data = raw_ptr->data;
    size_t i = 0;
    for (; i < counts - 1; ++i, ++node_id)
    {
        data[i].id = node_id;
        data[i].data = NULL;
        data[i].free_func = NULL;
        data[i].next = &data[i+1];
    }
    data[i].id = node_id;
    data[i].data = NULL;
    data[i].free_func = NULL;
    data[i].next = NULL;
    ++node_id;
    --handle_node_id;
    return raw_ptr;
}

void free_node(CMM_handle* cmm_handle, Node* node_ptr)
{
    if (node_ptr->free_func)
        node_ptr->free_func(node_ptr->data);
    else
        cmm_handle->free_func(node_ptr->data);
}

void free_hash_table(CMM_handle* cmm_handle)
{
    size_t i = 0;
    Node *temp_node_ptr = NULL;
    for (; i < cmm_handle->buckets; ++i)
    {
        temp_node_ptr = cmm_handle->hash_table[i];
        while (temp_node_ptr)
        {
            free_node(cmm_handle, temp_node_ptr);
            temp_node_ptr = temp_node_ptr->next;
        }
    }
    free(cmm_handle->hash_table);
}

void recycle_node(CMM_handle* cmm_handle, Node* node_ptr)
{
    node_ptr->next = cmm_handle->free_mem_stack;
    cmm_handle->free_mem_stack = node_ptr;
}

void free_mem_chain(CMM_handle* cmm_handle)
{
    Node *temp_node_ptr = cmm_handle->mem;
    while (temp_node_ptr)
    {
        cmm_handle->mem = cmm_handle->mem->next;
        free(temp_node_ptr);
        temp_node_ptr = cmm_handle->mem;
    }
}

int get_one_free_node(CMM_handle* cmm_handle, Node** new_node)
{
    if (cmm_handle->free_mem_stack == NULL)
    {
        cmm_handle->mem_tail->next = make_chunk_nodes(0);
        if (cmm_handle->mem_tail->next == NULL)
            return 1;
        cmm_handle->mem_tail = cmm_handle->mem_tail->next;
        cmm_handle->free_mem_stack = cmm_handle->mem_tail->data;
    }
    *new_node = cmm_handle->free_mem_stack;
    cmm_handle->free_mem_stack = cmm_handle->free_mem_stack->next;
    (*new_node)->data = NULL;
    (*new_node)->free_func = NULL;
    (*new_node)->next = NULL;
    return 0;
}

int register_memory(CMM_handle* cmm_handle, void *data,
        CMM_free_func free_func)
{
    Node *new_node = NULL;
    int error_code = get_one_free_node(cmm_handle, &new_node);
    if (error_code)
        return error_code;
    new_node->data = data;
    new_node->free_func = free_func;
    const size_t hash_index = cmm_handle->hash_func(cmm_handle, data);
    Node *scanner = cmm_handle->hash_table[hash_index], *prev = NULL;
    while (scanner != NULL)
    {
        prev = scanner;
        scanner = scanner->next;
    }
    if (prev == NULL)
        cmm_handle->hash_table[hash_index] = new_node;
    else
        prev->next = new_node;
    return 0;
}

size_t simple_hash(CMM_handle *cmm_handle, void *data)
{
    return ((size_t)data / cmm_handle->hash_factor % cmm_handle->buckets);
}
