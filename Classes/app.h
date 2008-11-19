
#include "cocoaconnect.h"

#include "jsstddef.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "jstypes.h"
#include "jsarena.h"
#include "jsutil.h"
#include "jsprf.h"
#include "jsapi.h"
#include "jsatom.h"
#include "jscntxt.h"
#include "jsdbgapi.h"
#include "jsemit.h"
#include "jsfun.h"
#include "jsgc.h"
#include "jslock.h"
#include "jsobj.h"
#include "jsparse.h"
#include "jsscope.h"

#include <CoreGraphics/CGGeometry.h>
#include <CoreGraphics/CGAffineTransform.h>

#define JSOBJECT_TO_OBJCID(cx,obj) (void*)JSVAL_TO_INT(	\
		OBJ_GET_SLOT(cx, obj, JSSLOT_PRIVATE)	)


extern JSClass app_class;

JSObject* app_register(
JSContext *cx,
JSObject *glob
);

JSBool __enumerate(JSContext *cx, JSObject *obj);
JSBool __resolve(JSContext *cx, JSObject *obj, jsval id, uintN flags,
				 JSObject **objp);


JS_STATIC_DLL_CALLBACK(JSBool)
_objc_method_invocation_wrapper(JSContext *cx, JSObject *obj,
                           uintN argc, jsval *argv, jsval *vp);
