
#import <UIKit/UIKit.h>

#import <objc/objc.h>
#import 	<objc/runtime.h>

#import "cocoaconnect.h"

#define SETTER_DEPRECATED 1


void* app_impl () {
	id app = [UIApplication sharedApplication];
	return (void*)app;
}

int sizeOfDataType(int type) {
	const char sizes[] = {
		0,	// dummy.
		sizeof(id),
		sizeof(int),
		sizeof(float),
		sizeof(CGAffineTransform),
		sizeof(CGRect),
		sizeof(CGPoint),
		sizeof(CGSize),
		0
	};
	return sizes[type];
}

char* jsMethodNameToObjcSelectorName(const char* js_method_name, char** lastColumn) {
	int length = strlen(js_method_name);
	int n = 0;

	char* objc_method_name = (char*)malloc(length + 2);

	const char* p = js_method_name;
	char* q = objc_method_name;

	char c;
	while (c = *p++) {
		if ( c == '_' ) {
			c = ':';
			n++;
		}
		*q++ = c;
	}
#if ! SETTER_DEPRECATED
	*lastColumn = q;
	*q++ = ':';
#endif
	*q = '\0';

	return objc_method_name;
}

#define __IF_TEXT_EQUAL(s)	\
	if ( strncmp(typeName, #s, n) == 0 ) {		\
		printf("type: "#s"\n");					\
		return CC_VAL_##s;						\
	}


static int signatureTextToType(const char* text) {
	if ( text && strcmp(text, "@") == 0 ) {
		printf("type: id\n");
		return CC_VAL_OBJC_ID;
	}

	const char* typeName = text; /*DONT RENAME THIS VARIABLE*/
	int n; /*DONT RENAME THIS VARIABLE*/
	
	const char* p = typeName;
	if ( *p == '{' ) {
		typeName = text + 1;
		const char* p = typeName;
		while (*p && *p != '=') {
			p++;
		}
		n = p - typeName;
	} else {
		n = 1;
	}

	__IF_TEXT_EQUAL(i);
	__IF_TEXT_EQUAL(f);
	__IF_TEXT_EQUAL(CGRect);
	__IF_TEXT_EQUAL(CGPoint);
	__IF_TEXT_EQUAL(CGSize);
	__IF_TEXT_EQUAL(CGAffineTransform);

	return CC_NOT_FONUD;
}


void* try_message(void* objc_id, const char* msg, char* done) {
	char* lastColumn;
	char* objc_method_name = jsMethodNameToObjcSelectorName(msg, &lastColumn);
	SEL method = sel_registerName(objc_method_name);

	id obj = (struct objc_object*)objc_id;
	if ( ! [obj respondsToSelector:method] ) {
#if SETTER_DEPRECATED
		free(objc_method_name);
		return NULL;
#else
		*lastColumn = '\0';
		method = sel_registerName(objc_method_name);
		if ( ! [obj respondsToSelector:method] ) {
			free(objc_method_name);
			return NULL;
		}
#endif
	}
	free(objc_method_name);

	SEL imsfs = ( @selector(methodSignatureForSelector:) ) ;
	if ( ! [obj respondsToSelector:imsfs] ) {
		return NULL;
	}
	
	// we need to check type of return value is derived from NSObject or not
	// before performSelector.
	NSMethodSignature* sig = [obj methodSignatureForSelector:method];
	int args = [sig numberOfArguments] ;
	// args includes 2 extra args(self and selector?).
	args -= 2;

	if ( args > 0 ) {
	//	const char** types = (const char**)malloc(args);
	//	for (int i = 0; i < args; i++) {
	//		types[i] = [sig getArgumentTypeAtIndex:2 + i];
	//	}

		OBJC_METHOD_INFO* info = (struct objc_method_info*)malloc(sizeof(struct objc_method_info));
		info->name = sel_getName(method);
		info->args = args;
	//	info->arg_types = types;

		*done = CC_MAKE_FUNCTION;
		return info;
	}

//	DOUBLE_TO_JSVAL
	
	int returnSize = [sig methodReturnLength];
	*done = signatureTextToType([sig methodReturnType]);

	NSInvocation* inv = [NSInvocation invocationWithMethodSignature:sig];
	[inv setTarget:obj];
	[inv setSelector:method];
	[inv invoke];
	void* returnValueBuffer = malloc(returnSize);
	[inv getReturnValue:returnValueBuffer];

	return returnValueBuffer;
}

const char* objcClassName(void* objc_id) {
	id obj = (struct objc_object*)objc_id;
	return class_getName( object_getClass(obj) );
}

void* send_message0(void* objc_id, const char* msg) {
	return NULL;
	
}
const char* get_description (void* objc_id) {
	id obj = (struct objc_object*)objc_id;
	NSString* desc = [obj description];
	return [desc cStringUsingEncoding:NSASCIIStringEncoding];
}

int cocoaconnect_validate_method_call(METHOD_CALL_CONTEXT* mccx) {
	id obj = (struct objc_object*)mccx->object;
	SEL method = sel_registerName(mccx->selector_name);
	NSMethodSignature* sig = [obj methodSignatureForSelector:method];
	int args = [sig numberOfArguments] ;
	args -= 2;

	if ( args != mccx->passed_args )
		return 0;
	
	mccx->signature = sig;
	return 1;
}

int cocoaconnect_argument_type(METHOD_CALL_CONTEXT* mccx, int n, int* size) {
	NSMethodSignature* sig = (NSMethodSignature*)mccx->signature;
	const char* typetext = [sig getArgumentTypeAtIndex:n + 2];

	unsigned char data_type = signatureTextToType(typetext);
	*size = sizeOfDataType(data_type);
	return data_type;
}

void* cocoaconnect_invoke(METHOD_CALL_CONTEXT* mccx) {
	id obj = (struct objc_object*)mccx->object;
	SEL method = sel_registerName(mccx->selector_name);
	NSMethodSignature* sig = (NSMethodSignature*)mccx->signature;

	NSInvocation* inv = [NSInvocation invocationWithMethodSignature:sig];
	[inv setTarget:obj];
	[inv setSelector:method];

	for (int i = 0; i < mccx->passed_args; i++) {
		void* p = mccx->args[i];
		[inv setArgument:p atIndex:i + 2];
	}
	[inv invoke];

	mccx->return_size = [sig methodReturnLength];
	mccx->return_type = signatureTextToType([sig methodReturnType]);

	if ( mccx->return_size <= 0 )
		return NULL;
	
	void* returnValueBuffer = malloc(mccx->return_size);
	[inv getReturnValue:returnValueBuffer];
	
	return returnValueBuffer;
}


int is_method_existing(void* objc_id, const char* selector_name) {
	SEL method = sel_registerName(selector_name);

	id obj = (struct objc_object*)objc_id;
	if ( [obj respondsToSelector:method] ) {
		return 1;
	} else {
		return 0;
	}
}

