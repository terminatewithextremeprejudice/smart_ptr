#ifndef SMART_PTR_
#define SMART_PTR_

/**
 * simple smart_ptr & unique_ptr implementation that uses 
 * GNU C extensions for dereferencing. 
 *
 * smart_ptr should be referenced by another another variable by smart_ptr_ref function
 * which increases the internal reference count. wheneber that that variable exits the scope of 
 * function according to compiler it will trigger smart_ptr_stack_free function 
 * which decreases refence count. with ref count being 0,
 * underlying ptr will be freed.
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#define MALLOC malloc
#define CALLOC calloc
#define REALLOC realloc
#define FREE free

#ifdef __GNUC__
# define MALLOC_API     __attribute__ ((MALLOC))
# define PURE           __attribute__ ((pure))
# define SECTION(name)  __attribute__ ((section("." #name "#")))
# if defined(__APPLE__)
#  define SECTION(name) __attribute__ ((section("__TEXT,__" #name)))
# endif
#else
# error("Unsupported compiler.")
#endif

static size_t
atomic_add(volatile size_t *count, const size_t limit, const size_t val)
{
	size_t old_count, new_count;

    	do {
      		old_count = *count;
      		if (old_count == limit)
          		abort();
      		new_count = old_count + val;
    	} while (!__sync_bool_compare_and_swap(count, old_count, new_count));
    	return new_count;
}

static void
atomic__store(size_t* p_ai, size_t val)
{
	int ai_was ;

    	ai_was = *p_ai ;
	ai_was = __sync_val_compare_and_swap (p_ai, ai_was, val);
}

#define atomic_fetch(ptr)			\
	(__sync_add_and_fetch(ptr, 0))		\

#define atomic_set(ptr, val)			\
	(atomic__store(ptr, val))		\

#define atomic_inc(count)               	\
	(atomic_add(count, SIZE_MAX, 1))    	\

#define atomic_dec(count)               	\
	(atomic_add(count, 0, -1))          	\

#define SENTINEL      .sentinel_ = 0,
#define SENTINEL_DEC int sentinel_;

enum smart_ptr_kind {
  UNIQUE,
  SHARED
};

typedef struct {
	enum smart_ptr_kind kind;
	void (*dtor)(void *, void *);
	void *ptr;
} smart_ptr_meta;

typedef struct {
	enum smart_ptr_kind kind;
	void (*dtor)(void *, void *);
	void *ptr;
	volatile size_t ref_count;
} smart_ptr_meta_shared;

typedef struct {
	const void *data;
	size_t      size;
} smart_ptr_malloc_meta;

typedef struct {
	SENTINEL_DEC
	size_t  size;
	size_t  nmemb;
	enum    smart_ptr_kind kind;
	void 	(*dtor)(void *, void *);
	smart_ptr_malloc_meta meta;
} smart_ptr_malloc_args;

void *smart_ptr_malloc(smart_ptr_malloc_args *args);
void *smart_ptr_ref(void *ptr);
void *smart_ptr_move_size(void *ptr, size_t size);
void  smart_ptr_stack_free(void *ptr);
void  smart_ptr_free(void *ptr);
void *smart_ptr_get_meta(void *ptr);

#define _smart_ptr_malloc(...) 						    \
  smart_ptr_malloc(&(smart_ptr_malloc_args) { SENTINEL __VA_ARGS__ })

#define smart_ptr_move(Ptr) 						    \
  smart_ptr_move_size((Ptr), sizeof (*(Ptr)))

#define ARGS_   args.dtor, { args.meta.data, args.meta.size }
#define smart   __attribute__ ((cleanup(smart_ptr_stack_free)))
#define smart_ptr(Kind, Type, ...)                                          \
    ({                                                                      \
        struct s_tmp {                                                      \
            SENTINEL_DEC                                                    \
            __typeof__(Type) value;                                         \
            void (*dtor)(void *, void *);                                   \
            smart_ptr_malloc_meta meta;                                     \
        } args = {                                                          \
            SENTINEL                                                        \
            __VA_ARGS__                                                     \
        };                                                                  \
        const __typeof__(Type[1]) dummy;                                    \
        void *var = sizeof (dummy[0]) == sizeof (dummy)                     \
            ? _smart_ptr_malloc(sizeof (Type), 0, Kind, ARGS_)               \
            : _smart_ptr_malloc(sizeof (dummy[0]),                           \
                    sizeof (dummy) / sizeof (dummy[0]), Kind, ARGS_);       \
        if (var != NULL)                                                    \
            memcpy(var, &args.value, sizeof (Type));                        \
        var;                                                                \
    })

#define shared_ptr(Type, ...) smart_ptr(SHARED, Type, __VA_ARGS__)
#define unique_ptr(Type, ...) smart_ptr(UNIQUE, Type, __VA_ARGS__)

inline size_t
align(size_t s)
{
	return (s + (sizeof (char *) - 1)) & ~(sizeof (char *) - 1);
}

PURE smart_ptr_meta *
get_meta(void *ptr)
{
  assert(ptr != NULL);
  size_t *size      = (size_t *) ptr - 1; 				/** at the beginning of size area */
  void   *ret_addr  = (smart_ptr_meta *)((char *) size - *size); 	/** at the beginning of the metadata */
  return (smart_ptr_meta *)ret_addr;
}


smart_ptr_meta_shared *
alloc_entry(size_t head_size, size_t size, size_t meta_size)
{
  const size_t total_size 	= head_size + size + meta_size + sizeof(size_t);
  smart_ptr_meta_shared *ptr 	= (smart_ptr_meta_shared *)MALLOC(total_size);
  assert(ptr != NULL);
  return ptr;
}

void
dealloc_entry(smart_ptr_meta *meta, void *ptr)
{
  assert(meta != NULL);
  assert(ptr != NULL);
  if (meta->dtor) {
    void* user_meta = smart_ptr_get_meta(ptr);
    meta->dtor(ptr, user_meta);
  }

  free(meta);
}

MALLOC_API void * 
smart_ptr_malloc(smart_ptr_malloc_args *args)
{
  assert(args != NULL);

  if (!args->size) {
    return NULL;
  }

  /* align the sizes to the size of a word */
  size_t aligned_meta_size  = align(args->meta.size);
  size_t size               = align(args->size);
  size_t head_size          = args->kind & SHARED ? sizeof(smart_ptr_meta_shared) : sizeof(smart_ptr_meta);

  smart_ptr_meta_shared* ptr = alloc_entry(head_size, size, aligned_meta_size);
  if (ptr == NULL) {
    return NULL;
  }

  char* shifted = (char *) ptr + head_size;
  if (args->meta.size && args->meta.data) {
    memcpy(shifted, args->meta.data, args->meta.size);
  }

  size_t* sz = (size_t *) ( shifted + aligned_meta_size);
  *sz = head_size + aligned_meta_size;

  *(smart_ptr_meta*)ptr = (smart_ptr_meta) {
    .kind = args->kind,
    .dtor = args->dtor,
    .ptr  = sz + 1
  };

  if (args->kind & SHARED) {
    ptr->ref_count = 1;
  }

  return sz + 1;
}

void *
smart_ptr_move_size(void *ptr, size_t size)
{
  assert(ptr != NULL);

  smart_ptr_malloc_args args;

  smart_ptr_meta *meta = get_meta(ptr);
  assert(meta->kind & UNIQUE);

  size_t *meta_size = (size_t *) ptr - 1;
  void   *user_meta = smart_ptr_get_meta(ptr);
  assert(user_meta != NULL);

  args = (smart_ptr_malloc_args) {
    .size = size,
    .kind = SHARED,
    .dtor = meta->dtor,
    .meta = { user_meta, *meta_size },
  };

  void *new_ptr = smart_ptr_malloc(&args);
  memcpy(new_ptr, ptr, size);

  return new_ptr;
}

void *
smart_ptr_get_meta(void* ptr)
{
  assert(ptr != NULL);
  assert((size_t)ptr == align((size_t)ptr));

  smart_ptr_meta* meta = get_meta(ptr);
  assert(meta->ptr == ptr);

  size_t head_size = meta->kind & SHARED ? sizeof(smart_ptr_meta_shared) : sizeof(smart_ptr_meta);
  size_t *meta_size = (size_t *) ptr - 1;
  assert(*meta_size == head_size);
  return meta;
}

void *
smart_ptr_ref(void *ptr)
{
  assert(ptr != NULL);
  smart_ptr_meta* meta = get_meta(ptr);
  assert(meta->ptr == ptr);
  assert(meta->kind & SHARED);
  atomic_inc(&((smart_ptr_meta_shared *) meta)->ref_count);
  return ptr;
}

void
smart_ptr_free(void *ptr)
{
  if (!ptr) {
    return;
  }


  assert((size_t) ptr == align((size_t)ptr));
  smart_ptr_meta* meta = get_meta(ptr);
  assert(meta->ptr == ptr);


  // check if meta is used on shared ptr and prevent dealloc if theres still references
  if (meta->kind & SHARED && atomic_dec(&((smart_ptr_meta_shared*) meta)->ref_count)) {
    return;
  }

  dealloc_entry(meta, ptr);
}

void
smart_ptr_stack_free(void *ptr)
{
  assert(ptr != NULL);
  union {
    void**  real_ptr;
    void*   ptr;
  } conv;

  conv.ptr = ptr;
  smart_ptr_free(*conv.real_ptr);
  *conv.real_ptr = NULL;
}

#endif /** SMART_PTR_H_ */

