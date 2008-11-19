//
//  uimonkeyViewController.m
//  uimonkey
//
//  Created by kuma on 08/11/18.
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#import "uimonkeyViewController.h"
#import "Httpd.h"

@implementation uimonkeyViewController



/*
// Override initWithNibName:bundle: to load the view using a nib file then perform additional customization that is not appropriate for viewDidLoad.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {
        // Custom initialization
    }
    return self;
}
*/

/*
// Implement loadView to create a view hierarchy programmatically.
- (void)loadView {
}
*/



// Implement viewDidLoad to do additional setup after loading the view.
- (void)viewDidLoad {
	httpd = [	[Httpd alloc] initWithPort:38880];

	SEL method = @selector(setAlpha:);
	NSMethodSignature* sig = [self methodSignatureForSelector:method];
	const char* typetext = [sig getArgumentTypeAtIndex:2];

    [super viewDidLoad];
}



- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
    // Release anything that's not essential, such as cached data
}

-(void)stopHttpd{
	[httpd release];
}

- (void)dealloc {
	[httpd release];
    [super dealloc];
}

@end
