
#include "app.h"

#include <CoreGraphics/CGGeometry.h>
#include <CoreGraphics/CGAffineTransform.h>

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
/*
static JSPropertySpec _props[] = {
	{"_id",			TID_OBJC_ID, JSPROP_READONLY, 0,0},
	{"windows",		TID_WINDOWS, JSPROP_ENUMERATE, 0,0},
	{"keyWindow",	TID_KEYWINDOW, JSPROP_ENUMERATE, 0,0},
	{0,0,0,0,0}
};
*/

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


JSObject* jsObjectWithObjcObject(JSContext* cx, JSObject* obj, const char* name, void* objc_id) {
	JSObject* prop = JS_DefineObject(cx, obj, name,
		&wrapper_class, NULL, JSPROP_PERMANENT);
	OBJ_SET_SLOT(cx, prop, JSSLOT_PRIVATE, INT_TO_JSVAL(objc_id));
	
	const char* method = "toString";
	JSAtom* atom = js_Atomize(cx, method, strlen(method), 0);
	if (!atom)
		return NULL;

	JSNative native = _objc_wrapper_to_string;

	JSFunction *f = JS_NewFunction(
		cx, native,
		0,  //args
		0, // flags
		NULL, 
		NULL // name
		);
	jsval to_string_func = OBJECT_TO_JSVAL(f->object);

	if (!OBJ_SET_PROPERTY(cx, prop, ATOM_TO_JSID(atom), &to_string_func)) {
		return NULL;
	}

	return prop;
}
jsval jsvalWithObjcObject(JSContext* cx, JSObject* obj, const char* name, void* objc_id) {
	return 
	OBJECT_TO_JSVAL(
	jsObjectWithObjcObject(cx, obj, name, objc_id)
					);
}
/*
JSBool _resolve(JSContext *cx, JSObject *obj, jsval id, uintN flags, JSObject **objp) {
	JSBool resolved;

	if (!JS_ResolveStandardClass(cx, obj, id, &resolved))
		return JS_FALSE;
	if (resolved) {
		*objp = obj;
		return JS_TRUE;
	}

	return JS_TRUE;
}
*/


#define SET_NUMBER_PROP(cx,obj,p_struct, name) \
	JS_NewNumberValue(cx, (jsdouble)p_struct->name, &v); \
	SET_PROP(cx, obj, #name, &v)


#define SET_PROP(cx,obj,name,jsvp) \
	OBJ_SET_PROPERTY(cx, obj, ATOM_TO_JSID(	\
		js_Atomize(cx, name, strlen(name), 0)	\
	), jsvp)

// TODO: generate from header files.

static JSObject* makeSize (JSContext* cx, const CGSize* p) {
	jsval	v;
	JSObject* obj = JS_NewObject(cx, NULL, NULL, NULL);

	SET_NUMBER_PROP(cx, obj, p, width);
	SET_NUMBER_PROP(cx, obj, p, height);

//	JS_NewNumberValue(cx, (jsdouble)p->width, &v);
//	SET_PROP(cx, obj, "width", &v);
//
//	JS_NewNumberValue(cx, (jsdouble)p->height, &v);
//	SET_PROP(cx, obj, "height", &v);
	
	return obj;
}

static JSObject* makePoint (JSContext* cx, const CGPoint* p) {
	jsval	v;
	JSObject* obj = JS_NewObject(cx, NULL, NULL, NULL);

	SET_NUMBER_PROP(cx, obj, p, x);
	SET_NUMBER_PROP(cx, obj, p, y);
	
	return obj;
}
static JSObject* makeCGRect (JSContext* cx, const CGRect* p) {
	jsval v;
	JSObject* obj = JS_NewObject(cx, NULL, NULL, NULL);

	JSObject* origin = makePoint(cx, &p->origin);
	JSObject* size = makeSize(cx, &p->size);

	v = OBJECT_TO_JSVAL(origin);
	SET_PROP(cx, obj, "origin", &v);
	v = OBJECT_TO_JSVAL(size);
	SET_PROP(cx, obj, "size", &v);

	return obj;
}
static JSObject* makeCGAffineTransform (JSContext* cx, const CGAffineTransform* p) {
	jsval	v;
	JSObject* obj = JS_NewObject(cx, NULL, NULL, NULL);
	SET_NUMBER_PROP(cx, obj, p, a);
	SET_NUMBER_PROP(cx, obj, p, b);
	SET_NUMBER_PROP(cx, obj, p, c);
	SET_NUMBER_PROP(cx, obj, p, d);
	SET_NUMBER_PROP(cx, obj, p, tx);
	SET_NUMBER_PROP(cx, obj, p, ty);
	return obj;
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

		char done = 0;
		void* ret = try_message(objc_id, msg, &done);
		if ( 0 ) {
		} else if ( done == CC_VAL_OBJC_ID ) {
			*vp = jsvalWithObjcObject(cx, obj, msg, ret);
		} else if ( done == CC_VAL_INT ) {
			unsigned int *v = (unsigned int*)ret;
			*vp = INT_TO_JSVAL(*v);
			free(ret);
		} else if ( done == CC_VAL_FLOAT ) {
			float *v = (float*)ret;
			JS_NewNumberValue(cx, (jsdouble)*v, vp);
			free(ret);
		} else if ( done == CC_VAL_CGAFFINETRANSFORM ) {
			CGAffineTransform *v = (CGAffineTransform*)ret;
			JSObject* t = makeCGAffineTransform(cx, v);
			*vp = OBJECT_TO_JSVAL(t);
			free(ret);
		} else if ( done == CC_VAL_CGRECT ) {
			CGRect *v = (CGRect*)ret;
			JSObject* t = makeCGRect(cx, v);
			*vp = OBJECT_TO_JSVAL(t);
			free(ret);
		} else if ( done == CC_VAL_CGPOINT ) {
		} else if ( done == CC_VAL_CGSIZE ) {
		}
	}

	return JS_TRUE;
}

static JSBool
_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
	printf( "setting its property %s,",
			JS_GetStringBytes(JS_ValueToString(cx, id)));
	printf( " new value %s\n",
			JS_GetStringBytes(JS_ValueToString(cx, *vp)));
	 return JS_TRUE;
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
	JSObject* obj = jsObjectWithObjcObject(cx, glob, "app", objc_id);
	return obj;
}

