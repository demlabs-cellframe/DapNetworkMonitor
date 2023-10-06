#import "DapNetworkMonitorIOS.h"


@implementation DapNetworkMonitorIOS

static void ReachabilityCallback(SCNetworkReachabilityRef target, SCNetworkConnectionFlags flags, void *info) {
    DapNetworkMonitorIOS *monitor = (__bridge DapNetworkMonitorIOS *)info;

    if (flags & kSCNetworkFlagsReachable) {
        // Network is reachable
        [monitor sigNetworkReachable];
    } else {
        // Network is not reachable
        [monitor sigNetworkNotReachable];
    }
}

+ (DapNetworkMonitorIOS *)sharedInstance {
    static DapNetworkMonitorIOS *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[DapNetworkMonitorIOS alloc] init];
    });
    return instance;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        SCNetworkReachabilityContext context = {0, (__bridge void *)self, NULL, NULL, NULL};
        self.reachabilityRef = SCNetworkReachabilityCreateWithAddress(NULL, (const struct sockaddr *)&zeroAddress);
        SCNetworkReachabilitySetCallback(self.reachabilityRef, ReachabilityCallback, &context);
        SCNetworkReachabilityScheduleWithRunLoop(self.reachabilityRef, CFRunLoopGetMain(), kCFRunLoopCommonModes);
    }
    return self;
}

- (void)dealloc {
    if (self.reachabilityRef != NULL) {
        SCNetworkReachabilityUnscheduleFromRunLoop(self.reachabilityRef, CFRunLoopGetMain(), kCFRunLoopCommonModes);
        CFRelease(self.reachabilityRef);
    }
}

- (BOOL)isNetworkReachable {
    SCNetworkConnectionFlags flags;
    if (SCNetworkReachabilityGetFlags(self.reachabilityRef, &flags)) {
        return (flags & kSCNetworkFlagsReachable);
    }
    return NO;
}

@end
