#ifndef CORE_SMART_PTR_
#define CORE_SMART_PTR_

/**
 * simple smart_ptr & unique_ptr implementation that uses 
 * GNU C extensions for derefering. 
 *
 * smart_ptr should be referenced by another another variable by smart_ptr_ref function
 * which increases the internal reference count. wheneber that that variable exits the scope of 
 * function according to compiler it will trigger smart_ptr_stack_free function 
 * which decreases refence count. with ref count being 0,
 * underlying ptr will be freed.
 */

#include <stdlib.h>
#include <string.h>

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
  void (*dtor)(void *, void *);
  smart_ptr_malloc_meta meta;
} smart_ptr_malloc_args;

void* smart_ptr_malloc(smart_ptr_malloc_args *args);
void* smart_ptr_ref(void *ptr);
void* smart_ptr_move_size(void *ptr, size_t size);
void  smart_ptr_stack_free(void *ptr);
void  smart_ptr_free(void *ptr);
void* smart_ptr_get_meta(void *ptr);

#define smart_ptr_malloc(...) \
  smart_ptr_malloc(&(smart_ptr_malloc_args) { SENTINEL __VA_ARGS__ })

#define smart_ptr_move(Ptr) \
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
            ? smart_ptr_malloc(sizeof (Type), 0, Kind, ARGS_)               \
            : smart_ptr_malloc(sizeof (dummy[0]),                           \
                    sizeof (dummy) / sizeof (dummy[0]), Kind, ARGS_);       \
        if (var != NULL)                                                    \
            memcpy(var, &args.value, sizeof (Type));                        \
        var;                                                                \
    })

#define shared_ptr(Type, ...) smart_ptr(SHARED, Type, __VA_ARGS__)
#define unique_ptr(Type, ...) smart_ptr(UNIQUE, Type, __VA_ARGS__)

#endif /** SMART_PTR_H_ */

