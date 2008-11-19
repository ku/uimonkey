
#include "app.h"
#include "converters.h";

#define __PROPERTY_BEGIN(T, bf, jsval_var)	\
	T* p = (T*)bf;								\
	JSObject* obj = JSVAL_TO_OBJECT(v);			\
	if ( !JSVAL_IS_OBJECT(v) ) return NULL;

#define __PROPERTY_DOUBLE(name)	\
	if ( !JS_GetProperty(cx, obj, #name, &v) ) return NULL;		\
	if ( JSVAL_IS_INT(v) )    p->name =  JSVAL_TO_INT(v);		\
	else if ( JSVAL_IS_DOUBLE(v) ) p->name = *JSVAL_TO_DOUBLE(v);	\
	else return NULL;

#define __PROPERTY_SUB_STRUCTURE(name, T)						\
	jsval name;												\
	if ( !JS_GetProperty(cx, obj, #name, &name) ) return NULL;	\
	__convert_to_##T(cx, name, &p->name, sizeof(T));

#define __PROPERTY_END	return p;
	

void* __convert_to_objc_id(JSContext* cx, jsval v, void* arg_buffer, int buffer_size) {
	void* objc_id = (void*)JSVAL_TO_INT(v);
	memcpy(arg_buffer, &objc_id, buffer_size);
	return arg_buffer;
}

void* __convert_to_int(JSContext* cx, jsval v, void* arg_buffer, int buffer_size) {
	int32 int_value;
	if ( !JS_ValueToInt32(cx, v, &int_value) )
		return NULL;

	memcpy(arg_buffer, &int_value, buffer_size);
	return arg_buffer;
}
void* __convert_to_float(JSContext* cx, jsval v, void* arg_buffer, int buffer_size) {

	JSType t = JS_TypeOfValue(cx,v);
	printf("t: %s", JS_GetTypeName(cx, t));

	jsdouble d;
	if (!JS_ValueToNumber(cx, v, &d))
		return NULL;

	float f = d;
	memcpy(arg_buffer, &f, buffer_size);
	return arg_buffer;
}
void* __convert_to_CGAffineTransform(JSContext* cx, jsval v, void* arg_buffer, int buffer_size) {
__PROPERTY_BEGIN(CGAffineTransform, arg_buffer, v)
	__PROPERTY_DOUBLE(a)
	__PROPERTY_DOUBLE(b)
	__PROPERTY_DOUBLE(c)
	__PROPERTY_DOUBLE(d)
	__PROPERTY_DOUBLE(tx)
	__PROPERTY_DOUBLE(ty)
__PROPERTY_END
}

void* __convert_to_CGRect(JSContext* cx, jsval v, void* arg_buffer, int buffer_size) {
__PROPERTY_BEGIN(CGRect, arg_buffer, v)
	__PROPERTY_SUB_STRUCTURE(origin, CGPoint)
	__PROPERTY_SUB_STRUCTURE(size, CGSize)
__PROPERTY_END
}
void* __convert_to_CGPoint(JSContext* cx, jsval v, void* arg_buffer, int buffer_size) {
__PROPERTY_BEGIN(CGPoint, arg_buffer, v)
	__PROPERTY_DOUBLE(x)
	__PROPERTY_DOUBLE(y)
__PROPERTY_END
}
void* __convert_to_CGSize(JSContext* cx, jsval v, void* arg_buffer, int buffer_size) {
__PROPERTY_BEGIN(CGSize, arg_buffer, v)
	__PROPERTY_DOUBLE(width)
	__PROPERTY_DOUBLE(height)
__PROPERTY_END
}
#define SET_NUMBER_PROP(cx,obj,p_struct, name) \
	jsval jsval##name;									\
	JS_NewNumberValue(cx, p->name, &jsval##name);		\
	JS_SetProperty(cx, obj, #name, &jsval##name);

// TODO: generate from header files.
JSObject* makeCGSize (JSContext* cx, const CGSize* p) {
	JSObject* obj = JS_NewObject(cx, NULL, NULL, NULL);
	SET_NUMBER_PROP(cx, obj, p, width);
	SET_NUMBER_PROP(cx, obj, p, height);
	return obj;
}

JSObject* makeCGPoint (JSContext* cx, const CGPoint* p) {
	JSObject* obj = JS_NewObject(cx, NULL, NULL, NULL);
	SET_NUMBER_PROP(cx, obj, p, x);
	SET_NUMBER_PROP(cx, obj, p, y);
	return obj;
}
JSObject* makeCGRect (JSContext* cx, const CGRect* p) {
	jsval v;
	JSObject* obj = JS_NewObject(cx, NULL, NULL, NULL);

	JSObject* origin = makeCGPoint(cx, &p->origin);
	JSObject* size = makeCGSize(cx, &p->size);

	v = OBJECT_TO_JSVAL(origin);
	JS_SetProperty(cx, obj, "origin", &v);
	v = OBJECT_TO_JSVAL(size);
	JS_SetProperty(cx, obj, "size", &v);

	return obj;
}
JSObject* makeCGAffineTransform (JSContext* cx, const CGAffineTransform* p) {
	JSObject* obj = JS_NewObject(cx, NULL, NULL, NULL);
	SET_NUMBER_PROP(cx, obj, p, a);
	SET_NUMBER_PROP(cx, obj, p, b);
	SET_NUMBER_PROP(cx, obj, p, c);
	SET_NUMBER_PROP(cx, obj, p, d);
	SET_NUMBER_PROP(cx, obj, p, tx);
	SET_NUMBER_PROP(cx, obj, p, ty);
	return obj;
}

void native_to_jsval (JSContext *cx, JSObject *obj, unsigned char data_type, void* data_buffer, jsval *vp) {
	if ( data_type == CC_VAL_i ) {
		unsigned int *v = (unsigned int*)data_buffer;
		*vp = INT_TO_JSVAL(*v);
	} else if ( data_type == CC_VAL_f ) {
		float *v = (float*)data_buffer;
		*vp = DOUBLE_TO_JSVAL((jsdouble)*v);
	} else if ( data_type == CC_VAL_CGAffineTransform ) {
		*vp = OBJECT_TO_JSVAL(makeCGAffineTransform(cx, (CGAffineTransform*)data_buffer));
	} else if ( data_type == CC_VAL_CGRect	) {
		*vp = OBJECT_TO_JSVAL(makeCGRect (cx, (CGRect*)data_buffer));
	} else if ( data_type == CC_VAL_CGPoint ) {
		*vp = OBJECT_TO_JSVAL(makeCGPoint(cx, (CGPoint*)data_buffer));
	} else if ( data_type == CC_VAL_CGSize ) {
		*vp = OBJECT_TO_JSVAL(makeCGSize (cx, (CGSize*)data_buffer));
	}

}


