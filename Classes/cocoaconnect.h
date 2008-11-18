

enum {
	CC_VAL_OBJC_ID = 1,
	CC_VAL_INT,
	CC_VAL_FLOAT,
	CC_VAL_CGAFFINETRANSFORM,
	CC_VAL_CGRECT,
	CC_VAL_CGPOINT,
	CC_VAL_CGSIZE
};

const char* objcClassName(void* objc_id) ;
void* app_impl () ;

void* try_message(void* objc_id, const char* msg, char* done) ;
void* send_message0(void*, const char*);
const char* get_description (void* objc_id) ;

	//[inv setArgument:(void*) atIndex: x + 2];
#define __DISPATCH_METHOD(TYPE_STR, T, OBJECT_CODE) \
	if ( strncmp(typeName, TYPE_STR, n) == 0 ) { \
		NSInvocation* inv = [NSInvocation invocationWithMethodSignature:sig];	\
		[inv setTarget:obj];	\
		[inv setSelector:method];	\
		[inv invoke];	\
		T * v = (T *)malloc(sizeof(T));	\
		[inv getReturnValue:v];	\
		*done = OBJECT_CODE;	\
		return v;	\
	}	\

