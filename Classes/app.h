
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


extern JSClass app_class;

JSObject* app_register(
JSContext *cx,
JSObject *glob
);

JSBool __enumerate(JSContext *cx, JSObject *obj);
JSBool __resolve(JSContext *cx, JSObject *obj, jsval id, uintN flags,
				 JSObject **objp);
