void native_to_jsval (JSContext *cx, JSObject *obj, unsigned char data_type, void* data_buffer, jsval *vp) ;

void* __convert_to_objc_id(JSContext* cx, jsval v, void* arg_buffer, int buffer_size) ;
void* __convert_to_int(JSContext* cx, jsval v, void* arg_buffer, int buffer_size);
void* __convert_to_float(JSContext* cx, jsval v, void* arg_buffer, int buffer_size);
void* __convert_to_CGAffineTransform(JSContext* cx, jsval v, void* arg_buffer, int buffer_size);
void* __convert_to_CGRect(JSContext* cx, jsval v, void* arg_buffer, int buffer_size);
void* __convert_to_CGPoint(JSContext* cx, jsval v, void* arg_buffer, int buffer_size);
void* __convert_to_CGSize(JSContext* cx, jsval v, void* arg_buffer, int buffer_size);


JSObject* makeCGSize (JSContext* cx, const CGSize* p) ;
JSObject* makeCGPoint (JSContext* cx, const CGPoint* p) ;
JSObject* makeCGRect (JSContext* cx, const CGRect* p) ;
JSObject* makeCGAffineTransform (JSContext* cx, const CGAffineTransform* p) ;


