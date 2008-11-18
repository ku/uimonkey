
#import <UIKit/UIKit.h>

#import <objc/objc.h>
#import 	<objc/runtime.h>

#import "cocoaconnect.h"

void* app_impl () {
	id app = [UIApplication sharedApplication];
	return (void*)app;
}

void* try_message(void* objc_id, const char* msg, char* done) {
	SEL method = sel_registerName(msg);

	id obj = (struct objc_object*)objc_id;
	if ( ! [obj respondsToSelector:method] )
		return NULL;

	SEL imsfs = ( @selector(methodSignatureForSelector:) ) ;
	if ( ! [obj respondsToSelector:imsfs] )
		return NULL;
	
	// we need to check type of return value is derived from NSObject or not
	// before performSelector.
	NSMethodSignature* sig = [obj methodSignatureForSelector:method];
	const char* returnType = [sig methodReturnType];
	if ( returnType && strcmp(returnType, "@") == 0 ) { 
		id ret = [obj performSelector:method];

		*done = CC_VAL_OBJC_ID;
		return (void*)ret;
	}

	const char* typeName = returnType; /*DONT RENAME THIS VARIABLE*/
	int n; /*DONT RENAME THIS VARIABLE*/
	
	if ( *returnType == '{' ) {
		typeName = returnType + 1;
		const char* p = typeName;
		while (*p && *p != '=') {
			p++;
		}
		n = p - typeName;
	} else {
		n = 1;
	}


	__DISPATCH_METHOD("i", unsigned int, CC_VAL_INT)
	__DISPATCH_METHOD("f", float, CC_VAL_FLOAT)
	__DISPATCH_METHOD("CGRect", CGRect, CC_VAL_CGRECT)
	//__DISPATCH_METHOD("CGAffineTransform", CGAffineTransform, CC_VAL_CGAFFINETRANSFORM)
	if ( strncmp(typeName, "CGAffineTransform", n) == 0 ) { 
		NSInvocation* inv = [NSInvocation invocationWithMethodSignature:sig];	
		[inv setTarget:obj];	
		[inv setSelector:method];	
		[inv invoke];	
		CGAffineTransform * v = (CGAffineTransform *)malloc(sizeof(CGAffineTransform));	
		[inv getReturnValue:v];	
		*done = CC_VAL_CGAFFINETRANSFORM;
		return v;
	}

	return NULL;
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

