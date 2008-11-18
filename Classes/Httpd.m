
#import "Httpd.h"
//#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "js.h"

@implementation Httpd
-(id)initWithPort:(int)n {
	if (self = [super init]) {
		[self _init_socket:n];
		js_init();
	}

	return self;
}

-(void)dealloc {
	close(server_socket);
	[super dealloc];
}

-(int)_init_socket:(int)n {
    struct sockaddr_in addr;
		
	int sock = socket(AF_INET, SOCK_STREAM, 0) ;
	server_socket = sock;
	
	int namelen = sizeof(addr);
	
	if(sock  <= 0 ) {
		return 1;
	}
				memset(&addr, 0, sizeof(addr));
	addr.sin_len = namelen;
	addr.sin_family = AF_INET;
				addr.sin_addr.s_addr = htonl(INADDR_ANY);
				addr.sin_port = htons(n);
				
				// Allow the kernel to choose a random port number by passing in 0 for the port.
				if (bind(sock, (struct sockaddr *)&addr, namelen) < 0) {
					close (sock);
					return 1;
				}
				/*
				// Find out what port number was chosen.
				if (getsockname(sock, (struct sockaddr *)&serverAddress, &namelen) < 0) {
					close(sock);
					return 2;
				}
				*/
				// Once we're here, we know bind must have returned, so we can start the listen
				if( listen(sock, 1) ) {
					close(sock);
					return 3;
				}

	
	NSFileHandle* fh = [[NSFileHandle alloc] initWithFileDescriptor:sock
													 closeOnDealloc:YES
	];

	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(observe:)
												 name:NSFileHandleConnectionAcceptedNotification
											   object:fh];
	[fh acceptConnectionInBackgroundAndNotify];

	return 0;
}

-(void)run {
	char* p = "app.keyWindow.frame";
	const char* res = js_run(p);

	fprintf(stdout, "%s\n", res);
}

-(void)observe:(NSNotification*)notification {
	NSLog(@"%@", notification);

		NSDictionary* info = [notification userInfo];
	NSFileHandle* fh = (NSFileHandle*)[info objectForKey:NSFileHandleNotificationFileHandleItem];
	
	NSData* data = [fh availableData];
	
	const char* p = (const char*)[data bytes];

	BOOL found = NO;

	if ( memcmp(p, "GET", 3) == 0) {
		p += 3;
		const char* start = ++p;
		while ( *p != ' ' )
			p++;
		int n = p - start;
		NSString* path = [[NSString alloc] initWithBytes:start
			length:n encoding:NSASCIIStringEncoding];
		NSLog(@"get %@", path);
	} else 	if ( memcmp(p, "POST", 4) == 0) {
		//NSString* s = [[NSString alloc] initWithData:data encoding:NSASCIIStringEncoding];
		const char* eoh = strstr(p, "\r\n\r\n");
		if ( eoh ) {
			eoh += 4;
			int n = [data length] - ( eoh - p);
			NSString *body = [[NSString alloc] initWithBytes:eoh
			length: n 
			encoding:NSASCIIStringEncoding];

			NSLog(@"body: %@", body);

			char* p = (char*)malloc(n + 1);
			p[n] = '\0';
			memcpy(p, eoh, n);
			const char* res = js_run(p);
			free(p);

			const char* header = "HTTP/1.0 200 OK\r\n\r\n";
			const char* nl = "\n";
			NSMutableData* d = [NSMutableData dataWithBytes:header length:strlen(header)];
			[d appendBytes:res length:strlen(res)];
			[d appendBytes:nl length:strlen(nl)];

			[fh writeData:d];
			//fprintf(stdout, "%s\n", res);
			//
			//
			found = YES;
		}
	}

	if ( !found ) {
		const char* res = "HTTP/1.0 404 Not Found\r\n\r\n";
		NSData* response = [NSData dataWithBytes:res length:strlen(res)];
		[fh writeData:response];
//		[fh synchronizeFile];
//		[fh closeFile];
	}	

	[[notification object] acceptConnectionInBackgroundAndNotify];


}



@end
