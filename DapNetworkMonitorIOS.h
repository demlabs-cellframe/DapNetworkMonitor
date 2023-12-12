#import "DapNetworkMonitorAbstract.h"
#import <CoreFoundation/CoreFoundation.h>
#import <SystemConfiguration/SystemConfiguration.h>
#import <Foundation/Foundation.h>

@interface DapNetworkMonitorIOS : DapNetworkMonitorAbstract

+ (DapNetworkMonitorIOS *)sharedInstance;

@end
