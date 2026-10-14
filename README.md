# smart_ptr
 
 simple smart_ptr & unique_ptr implementation that uses GNU C extensions for derefering. 
smart_ptr should be reference to another another variable by smart_ptr_ref function
which increases the internal reference count. wheneber that that variable exits the scope of 
function according to compiler it will trigger smart_ptr_stack_free function 
which decreases refence count. with ref count being 0,
underlying ptr will be freed.
