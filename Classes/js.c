
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

#include "js.h";
#include "app.h";

extern JSClass global_class;


JSBool __enumerate(JSContext *cx, JSObject *obj)
{
	JSBool b;

	return !b || JS_EnumerateStandardClasses(cx, obj);
}
JSBool __resolve(JSContext *cx, JSObject *obj, jsval id, uintN flags,
               JSObject **objp) {
	JSBool b, resolved;

	if (b && (flags & JSRESOLVE_ASSIGNING) == 0) {
		if (!JS_ResolveStandardClass(cx, obj, id, &resolved))
			return JS_FALSE;
		if (resolved) {
			*objp = obj;
			return JS_TRUE;
		}
	}
	*objp = NULL;
	return JS_TRUE;
}

JSClass global_class = {
	"global", JSCLASS_NEW_RESOLVE | JSCLASS_GLOBAL_FLAGS,
	JS_PropertyStub,  JS_PropertyStub,
	JS_PropertyStub,  JS_PropertyStub,
	__enumerate, (JSResolveOp) __resolve,
	JS_ConvertStub,   JS_FinalizeStub,
	JSCLASS_NO_OPTIONAL_MEMBERS
};


size_t gStackChunkSize = 8192;

static void
my_ErrorReporter(JSContext *cx, const char *message, JSErrorReport *report)
{
	printf("error");
}

extern JSClass global_class;

JSRuntime *rt;
JSContext *cx;
JSObject *glob;

int js_init() {

	 rt = JS_NewRuntime(1L * 1024L * 1024L);

	 if (!rt)
		 return 1;

	 cx = JS_NewContext(rt, gStackChunkSize);
	 if (!cx)
		 return 1;

	 JS_SetErrorReporter(cx, my_ErrorReporter);

	 glob = JS_NewObject(cx, &global_class, NULL, NULL);
	 if (!glob)
		 return 1;

	 if (!JS_InitStandardClasses(cx, glob))
		 return 1;
	
	app_register(cx, glob);

	return 0;
}

const char* js_run(const char* buffer) {
	JSObject* obj = glob;

	int startline = 1;
//	 if (!JS_DefineFunctions(cx, glob, shell_functions))
//		 return 1;
	JSScript * script = JS_CompileScript(cx, obj, buffer, strlen(buffer), "typein",
			 startline);

			  jsval result;
			  JSBool ok;
	  ok = JS_ExecuteScript(cx, obj, script, &result);

	 JSString *str = JS_ValueToString(cx, result);
	 const char * p = JS_GetStringBytes(str);
	 int n = strlen(p);
	 char* ret = malloc(n+1);
	 memcpy(ret, p, n);
	 ret[n] = 0;
	 JS_DestroyScript(cx, script);
/*
	  if (str) {
		  fprintf(stdout, "%s\n", JS_GetStringBytes(str));
	  } else
	    JS_DestroyScript(cx, script);
*/
	return ret;
}

