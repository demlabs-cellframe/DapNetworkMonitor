#import "DapNetworkMonitorAbstract.h"
#import <CoreFoundation/CoreFoundation.h>
#import <SystemConfiguration/SystemConfiguration.h>

@interface DapNetworkMonitorIOS : DapNetworkMonitorAbstract

+ (DapNetworkMonitorIOS *)sharedInstance;

@end
