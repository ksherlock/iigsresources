#import <Foundation/Foundation.h>

#import "IIgsResource.h"

#include <stdio.h>
#include <stdlib.h>

void usage(int exitCode)
{
    fprintf(exitCode == 0 ? stdout : stderr, "Usage: %s resource file [...]\n", getprogname());
    exit(exitCode);
}


void rlist(char *file)
{

    unsigned count;
    unsigned i;
    
    IIgsResource *resource = [IIgsResource resourceWithFile: [NSString stringWithUTF8String: file]];

    if (!resource)
    {
        fprintf(stderr, "invalid resource file: ``%s''\n", file);
        return;
    }
    
    printf("%s\n", file);
    printf("Type   ID         Attr   Size\n");
    printf("-----  ---------  -----  ---------  --------\n");
    
    count = [resource countResources];
    for (i = 0; i < count; ++i)
    {
        ResRefRec r = [resource resourceAtIndex: i];
        
        printf("$%04x  $%08x  $%04x  $%08x  %8u\n", r.resType, r.resID, r.resAttr, r.resSize, r.resSize);  
    }
    printf("\n");

    NSLog(@"%@", [resource stringForID: 0x001 type: rPString]);

}

int main(int argc, char **argv)
{
    unsigned i;
    
    if (argc < 2) usage(1);

    for (i = 1; i < argc; ++i)
    {
        NSAutoreleasePool *pool = [NSAutoreleasePool new];
        rlist(argv[i]);
        [pool drain];
    } 
    
    exit(0);
}
