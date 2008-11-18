
#import <Foundation/Foundation.h>


@interface Httpd : NSObject  {
	int server_socket;
}
-(id)initWithPort:(int)n ;
-(int)_init_socket:(int)n ;
//-(int)_init_js;
-(void)observe:(NSNotification*)notification ;

-(void)run;

@end

