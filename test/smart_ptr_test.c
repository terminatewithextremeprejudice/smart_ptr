/** tests for tag types at their default configuration */

#include "../smart_ptr.h"

#include <unistd.h>
#include <assert.h>

void 
referencing_function(int *a)
{
	int *b = smart_ptr_ref(a);
	return;
}

void 
dereferencing_function(int *a)
{
	smart int *b = smart_ptr_ref(a);
	return;
}

int 
main(void)
{
	int *shared_int = shared_ptr(int, 1);

  	smart_ptr_meta_shared* m = smart_ptr_get_meta(shared_int);

  	*((int*)m->ptr) = 2;
  	assert(*((int*)m->ptr) == 2);
	assert(m->ref_count == 1);

  	smart int *reference = smart_ptr_ref(shared_int);
  	assert(reference != NULL);
	assert(m->ref_count == 2); 

  	*reference = 666;
	assert(*reference == *((int*)m->ptr));

	referencing_function(shared_int);
	assert(m->ref_count == 3);
	dereferencing_function(shared_int);
	assert(m->ref_count == 3);

	return 0;
}
