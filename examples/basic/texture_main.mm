
#include "texture_main.cpp"

#include <iostream>

static bool fileExists(NSString *path) {
    return path && [[NSFileManager defaultManager] fileExistsAtPath:path];
}

int main() {
    NSFileManager *fm = [NSFileManager defaultManager];
    NSBundle *bundle = [NSBundle mainBundle];
    NSString *libraryPath = nil;

    do {
        NSString* resourcesDir = nil;
        
        // 1. Contents/MacOS/resources (cross-platform runtime assets)
        NSString *executablePath = [bundle executablePath];
        if (executablePath) {
            NSString *macOSDir =
            [executablePath stringByDeletingLastPathComponent];
            resourcesDir = [macOSDir stringByAppendingPathComponent:@"resources"];
            
            if (fileExists(resourcesDir)) {
                libraryPath = resourcesDir;
                break;
            }
        }
        break;
        
        // 2. Sibling to the .app bundle
        NSString *bundlePath = [bundle bundlePath];
        if (bundlePath) {
            NSString *bundleParent =
                [bundlePath stringByDeletingLastPathComponent];
            resourcesDir = [bundleParent stringByAppendingPathComponent:@"resources"];

            if (fileExists(resourcesDir)) {
                libraryPath = resourcesDir;
                break;
            }
        }

        // 3. Current working directory
        NSString *cwd = [fm currentDirectoryPath];
        resourcesDir = [cwd stringByAppendingPathComponent:@"resources"];

        if (fileExists(resourcesDir)) {
            libraryPath = resourcesDir;
            break;
        }

        std::cerr << "`resources` dir not found in bundle or cwd." << std::endl;
        return 1;
    } while (true);

    std::string resourcesPath([libraryPath fileSystemRepresentation]);
    return run_app(resourcesPath.c_str());
}
