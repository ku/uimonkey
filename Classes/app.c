
#include "app.h"

#include <CoreGraphics/CGGeometry.h>
#include <CoreGraphics/CGAffineTransform.h>

#include "converters.h";


static JSBool _getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
static JSBool _setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
//static JSBool _resolve(JSContext *cx, JSObject *obj, jsval id, uintN flags, JSObject **objp) ;

static JSBool
_convert(JSContext *cx, JSObject *obj, JSType type, jsval *vp);


JSClass wrapper_class = {
	"ObjcWrapper", JSCLASS_NEW_RESOLVE | JSCLASS_HAS_PRIVATE,
	JS_PropertyStub,  JS_PropertyStub,
	_getProperty,
	_setProperty,
	__enumerate, (JSResolveOp) __resolve,
	_convert,   JS_FinalizeStub,
	JSCLASS_NO_OPTIONAL_MEMBERS
};


enum string_tinyid {
	TID_OBJC_ID			= -1,
	TID_TRANSFORM		= -2,
	TID_WINDOWS			= -3,
	TID_KEYWINDOW		= -4
};

void* (*__converters[])(JSContext*, jsval, void*, int) = {
	NULL,
	__convert_to_objc_id,
	__convert_to_int,
	__convert_to_float,
	__convert_to_CGAffineTransform,
	__convert_to_CGRect,
	__convert_to_CGPoint,
	__convert_to_CGSize,
	NULL
};

JS_STATIC_DLL_CALLBACK(JSBool)
_objc_wrapper_to_string(JSContext *cx, JSObject *obj,
                           uintN argc, jsval *argv, jsval *vp)
{
	char namebuffer[128];
	
	void* objc_id = (void*)JSVAL_TO_INT(
		OBJ_GET_SLOT(cx, obj, JSSLOT_PRIVATE)
	);

	const char* classname = objcClassName(objc_id);

	if ( strcmp(classname, "NSCFString") == 0 ) {
		const char* description = get_description(objc_id);
		sprintf(namebuffer, "[objcObject %s] %s", classname,
			description
		);
	} else {
		sprintf(namebuffer, "[objcObject %s]", classname);
	}
	
	//int n = strlen(p);
	JSString* s = JS_NewStringCopyZ(cx, namebuffer);
	*vp = STRING_TO_JSVAL(s);
	return JS_TRUE;
}

static JSBool __invoke_objc_method(const char* selector_name, JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *vp) {
	METHOD_CALL_CONTEXT* mccx = (METHOD_CALL_CONTEXT*)malloc(sizeof(METHOD_CALL_CONTEXT));
	mccx->object = JSOBJECT_TO_OBJCID(cx, obj);
	mccx->selector_name = selector_name;
	mccx->passed_args = argc;
	
	if ( ! cocoaconnect_validate_method_call(mccx) )
		return JS_FALSE;
	
	void** args = (void*)malloc(argc);
	memset(args, 0, argc);
	mccx->args = args;
	
	for (int i = 0; i < argc; i++) {
		int size;
		int type = cocoaconnect_argument_type(mccx, i, &size);

		void * arg_buffer = malloc(size);

		void* arg = __converters[type](cx, argv[i], arg_buffer, size);
		if ( arg == NULL ) {
			args[i] = arg_buffer;
			goto cleanup;
		}
		args[i] = arg;
	}

	void* ret = cocoaconnect_invoke(mccx);

	// ret is NULL if the method returns NULL.

	if ( ret ) {
		native_to_jsval(cx, obj, mccx->return_type, ret, vp);
	}
cleanup:
	for (int i = 0; i < argc; i++) {
		void* p = (mccx->args[i]);
		if ( p )
			free(p);
	}
	free(args);
	return JS_TRUE;
}


JS_STATIC_DLL_CALLBACK(JSBool)
_objc_method_invocation_wrapper(JSContext *cx, JSObject *obj,
                           uintN argc, jsval *argv, jsval *vp)
{
	jsval funval = (argv[-2]);

	jsval v;
	if ( !JS_GetProperty(cx, JSVAL_TO_OBJECT( funval), "name", &v) ) {
		return JS_FALSE;
	}
	const char* selector_name = JS_GetStringBytes(JSVAL_TO_STRING(v));

	return __invoke_objc_method(
		selector_name, 
		cx, obj, argc, argv, vp
	);
}

static JSObject* wrapObjcObject(JSContext* cx, JSObject* obj, const char* name, void* objc_id) {
	JSObject* prop = JS_DefineObject(cx, obj, name, &wrapper_class, NULL, JSPROP_PERMANENT);

	// keeping objc object pointer.
	OBJ_SET_SLOT(cx, prop, JSSLOT_PRIVATE, INT_TO_JSVAL(objc_id));
	
	// overwrite toString to show Objective-C class name.
	const char* method = "toString";
	JSAtom* atom = js_Atomize(cx, method, strlen(method), 0);
	if (!atom)
		return NULL;

	JSFunction *f = JS_NewFunction(
		cx, _objc_wrapper_to_string,
		0,  //args
		0, // flags
		NULL, 
		NULL // name
	);
	jsval to_string_func = OBJECT_TO_JSVAL(f->object);
	if (!OBJ_SET_PROPERTY(cx, prop, ATOM_TO_JSID(atom), &to_string_func)) {
		return NULL;
	}

	jsval address = INT_TO_JSVAL( (unsigned int)objc_id );
	JS_SetProperty(cx, obj, "__objc_addr", &address);

	return prop;
}

static JSBool _getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
	if (JSVAL_IS_INT(id)) {
		//
	} else if ( JSVAL_IS_STRING(id) ) {
		void* objc_id = (void*)JSVAL_TO_INT(
			OBJ_GET_SLOT(cx, obj, JSSLOT_PRIVATE)
		);
		const char * msg = JS_GetStringBytes(JSVAL_TO_STRING(id));

		char done = CC_NOT_FONUD;
		void* ret = try_message(objc_id, msg, &done);
		if ( done == CC_NOT_FONUD ) {
		} else if ( done == CC_VAL_OBJC_ID ) {
			void* objc_id = *((void **)ret);
			*vp = OBJECT_TO_JSVAL( wrapObjcObject(cx, obj, msg, objc_id) );
			free(ret);
		} else {
			if ( done == CC_MAKE_FUNCTION ) {
				OBJC_METHOD_INFO* info = (OBJC_METHOD_INFO*)ret;
				JSFunction *fun = JS_NewFunction(
					cx, _objc_method_invocation_wrapper,
					info->args,  //number of args
					0, // flags
					NULL, // parent object
					info->name
				);
				jsval f = OBJECT_TO_JSVAL( fun->object );
				JS_SetProperty(cx, obj, msg, &f);
				*vp = f;
			} else {
				native_to_jsval(cx, obj, done, ret, vp);
			}
			free(ret);
		}
	}

	return JS_TRUE;
}



static JSBool
_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
	if ( !JSVAL_IS_STRING(id) ) {
		return JS_TRUE;
	}

	void* objc_id = JSOBJECT_TO_OBJCID(cx, obj);
	const char * msg = JS_GetStringBytes(JSVAL_TO_STRING(id));

	char firstChar = msg[0];
	if ( !isalpha(firstChar)  )
		return JS_TRUE;
	firstChar &= ~0x20;

	char setter_name[256];
	sprintf(setter_name, "set%c%s:", firstChar, (msg+1));

	int exsiting = is_method_existing(objc_id, setter_name);
	if ( exsiting == 0 )
		return JS_TRUE;

	int argc = 1;
	jsval* argv = (jsval*)malloc(sizeof(jsval) * argc);
	argv[0] = *vp;

	return __invoke_objc_method( setter_name, cx, obj, argc, argv, vp);
}

static JSBool
_convert(JSContext *cx, JSObject *obj, JSType type, jsval *vp)
{
	return JS_TRUE;
}
/*
static JSBool
App(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	jsdouble d;
	jsval v;

	if (argc != 0) {
		if (!js_ValueToNumber(cx, argv[0], &d))
			return JS_FALSE;
	} else {
		d = 0.0;
	}
	if (!js_NewNumberValue(cx, d, &v))
		return JS_FALSE;
	if (!(cx->fp->flags & JSFRAME_CONSTRUCTING)) {
		*rval = v;
		return JS_TRUE;
	}
	OBJ_SET_SLOT(cx, obj, JSSLOT_PRIVATE, v);
	return JS_TRUE;
}
*/

JSObject* app_register( JSContext *cx, JSObject *glob) {
	void* objc_id = app_impl();
	JSObject* obj = wrapObjcObject(cx, glob, "app", objc_id);
	return obj;
}

