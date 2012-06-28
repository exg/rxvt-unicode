#ifndef EMMAN_H_
#define EMMAN_H_

#include <stddef.h>

#if __cplusplus
extern "C" {
#endif

size_t chunk_round (size_t size);
size_t chunk_fit (size_t header, size_t element_size, size_t max_increase);
void *chunk_alloc (size_t size, int populate);
void *chunk_realloc (void *ptr, size_t old_size, size_t new_size);
void chunk_free (void *ptr, size_t size);

#if __cplusplus
}
#endif

#endif

