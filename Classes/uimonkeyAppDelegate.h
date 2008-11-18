//
//  uimonkeyAppDelegate.h
//  uimonkey
//
//  Created by kuma on 08/11/18.
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#import <UIKit/UIKit.h>

@class uimonkeyViewController;

@interface uimonkeyAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    uimonkeyViewController *viewController;
	
	
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet uimonkeyViewController *viewController;

@end

