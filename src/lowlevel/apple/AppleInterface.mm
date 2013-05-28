/*
 * Copyright (C) 2006-2013 Christopho, Solarus - http://www.solarus-games.org
 *
 * Solarus is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Solarus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <lowlevel/apple/AppleInterface.h>

#if defined(SOLARUS_OSX)
#  import <Cocoa/Cocoa.h>
#elif defined(SOLARUS_IOS)
#  import <Foundation/Foundation.h>
#  import <UIKit/UIKit.h>
#endif


/**
 * @brief Return "~/Library/Application Support/" or equivalent from the official way, which is available in OSX 10.6+ and iOS 4.0+.
 *
 * Return an OSX 10.0+ and iOS 1.0+ (not compatible with Iphone Simulator) hardcoded equivalent workaround if URLsForDirectory:inDomains: is not found.
 *
 * @return The Application Support folder from the User Domain
 */
const char* getUserApplicationSupportDirectory()
{
#if (defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MIN_ALLOWED >= MAC_OS_X_VERSION_10_6) || (defined(__IPHONE_4_0) && __IPHONE_OS_VERSION_MIN_ALLOWED >= __IPHONE_4_0)
    return [[[[[NSFileManager defaultManager] 
               URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask] 
              objectAtIndex:0] 
             path] 
            UTF8String];
#else
    // WORKAROUND : Avoid to report errors with undefined enum, and warning with undefined functions.
    // ( enum NSApplicationSupportDirectory and NSUserDomainMask are not defined on older OSX frameworks )
    return [[NSHomeDirectory() stringByAppendingPathComponent:@"Library/Application Support/"] 
            UTF8String];
#endif
}
