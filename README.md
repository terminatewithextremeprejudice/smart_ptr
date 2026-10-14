# smart_ptr
 
## DOCUMENTATION

 simple smart_ptr & unique_ptr implementation that uses GNU C extensions for dereferencing. 
smart_ptr should be referenced by another another variable by smart_ptr_ref function
which increases the internal reference count. wheneber that that variable exits the scope of 
function according to compiler it will trigger smart_ptr_stack_free function 
which decreases refence count. with ref count being 0,
underlying ptr will be freed.

Usage example
```
int *shared_int = shared_ptr(int, 1);
smart int *reference = smart_ptr_ref(shared_int); // smart attribute free's the reference from stack when the variable out of scope of the function 
smart_ptr_meta_shared* m = smart_ptr_get_meta(shared_int); // referencing to meta object increases reference count too 
assert(m->ref_count == 2);
return 0; // m->ref_count == 0
```
