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
  	smart_ptr_meta_shared* m = smart_ptr_get_meta(b);
	assert(m->ref_count == 4);
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

	smart int *reference2 = smart_ptr_ref(reference);
	assert(m->ref_count == 4);
	smart_ptr_meta_shared* m2 = smart_ptr_get_meta(reference2);
	assert(m2->ref_count == m->ref_count);

	smart int *shared_int2 = shared_ptr(int, 999);
	smart_ptr_meta_shared* m3 = smart_ptr_get_meta(shared_int2);
	assert(m3->ref_count == 1);
	*shared_int2 = *shared_int;
	assert(m3->ref_count == 1);
	assert(*shared_int2 == *shared_int);


	return 0;

}
