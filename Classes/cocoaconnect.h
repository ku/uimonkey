
typedef struct objc_method_info {
	int args;
	const char* name;
	//const char* arg_types[];
} OBJC_METHOD_INFO;

typedef struct objc_method_call_context {
	void*			object;
	void*			signature;	// NSMethodSignature
	const char*		selector_name;
	int				passed_args;
	void**			args;
	unsigned char	return_type;
	unsigned int	return_size;
} METHOD_CALL_CONTEXT;

enum {
	CC_NOT_FONUD = -1,
	CC_VAL_OBJC_ID = 1,
	CC_VAL_i,
	CC_VAL_f,
	CC_VAL_CGAffineTransform,
	CC_VAL_CGRect,
	CC_VAL_CGPoint,
	CC_VAL_CGSize,
	CC_MAKE_FUNCTION
};
int sizeOfDataType(int type);


const char* objcClassName(void* objc_id) ;
void* app_impl () ;

int is_method_existing(void* objc_id, const char* selector_name) ;
void* try_message(void* objc_id, const char* msg, char* done) ;
void* send_message0(void*, const char*);
const char* get_description (void* objc_id) ;


int cocoaconnect_validate_method_call(METHOD_CALL_CONTEXT* mccx);
	//[inv setArgument:(void*) atIndex: x + 2];
int cocoaconnect_argument_type(METHOD_CALL_CONTEXT* mccx, int n, int* size) ;
void* cocoaconnect_invoke(METHOD_CALL_CONTEXT* mccx);


