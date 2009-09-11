//
//  IIgsResource.h
//  IIgsResource
//
//  Created by Kelvin Sherlock on 9/9/2009.
//

#import <Foundation/Foundation.h>

#include <stdint.h>

enum {
    rIcon               = 0x8001,       /* Icon type */
    rPicture            = 0x8002,       /* Picture type */
    rControlList        = 0x8003,       /* Control list type */
    rControlTemplate    = 0x8004,       /* Control template type */
    rC1InputString      = 0x8005,       /* GS/OS class 1 input string */
    rPString            = 0x8006,       /* Pascal string type */
    rStringList         = 0x8007,       /* String list type */
    rMenuBar            = 0x8008,       /* MenuBar type */
    rMenu               = 0x8009,       /* Menu template */
    rMenuItem           = 0x800A,       /* Menu item definition */
    rTextForLETextBox2  = 0x800B,       /* Data for LineEdit LETextBox2 call */
    rCtlDefProc         = 0x800C,       /* Control definition procedure type */
    rCtlColorTbl        = 0x800D,       /* Color table for control */
    rWindParam1         = 0x800E,       /* Parameters for NewWindow2 call */
    rWindParam2         = 0x800F,       /* Parameters for NewWindow2 call */
    rWindColor          = 0x8010,       /* Window Manager color table */
    rTextBlock          = 0x8011,       /* Text block */
    rStyleBlock         = 0x8012,       /* TextEdit style information */
    rToolStartup        = 0x8013,       /* Tool set startup record */
    rResName            = 0x8014,       /* Resource name */
    rAlertString        = 0x8015,       /* AlertWindow input data */
    rText               = 0x8016,       /* Unformatted text */
    rCodeResource       = 0x8017,
    rCDEVCode           = 0x8018,
    rCDEVFlags          = 0x8019,
    rTwoRects           = 0x801A,       /* Two rectangles */
    rFileType           = 0x801B,       /* Filetype descriptors--see File Type Note $42 */
    rListRef            = 0x801C,       /* List member */
    rCString            = 0x801D,       /* C string */
    rXCMD               = 0x801E,
    rXFCN               = 0x801F,
    rErrorString        = 0x8020,       /* ErrorWindow input data */
    rKTransTable        = 0x8021,       /* Keystroke translation table */
    rWString            = 0x8022,       /* not useful--duplicates $8005 */
    rC1OutputString     = 0x8023,       /* GS/OS class 1 output string */
    rSoundSample        = 0x8024,
    rTERuler            = 0x8025,       /* TextEdit ruler information */
    rFSequence          = 0x8026,
    rCursor             = 0x8027,       /* Cursor resource type */
    rItemStruct         = 0x8028,       /* for 6.0 Menu Manager */
    rVersion            = 0x8029,
    rComment            = 0x802A,
    rBundle             = 0x802B,
    rFinderPath         = 0x802C,
    rPaletteWindow      = 0x802D,       /* used by HyperCard IIgs 1.1 */
    rTaggedStrings      = 0x802E,
    rPatternList        = 0x802F,
    rRectList           = 0xC001,
    rPrintRecord        = 0xC002,
    rFont               = 0xC003

};

typedef struct ResRefRec {
    unsigned resType;
    unsigned resID;
    unsigned resOffset;
    unsigned resAttr;
    unsigned resSize;
    
    unsigned resIndex;
    
} ResRefRec;



@interface IIgsResource : NSObject {

    unsigned _size;
    uint8_t *_resource;
    void *_map;
    void *_types;
}


+(id)resourceWithFile: (NSString *)filename;
-(id)initWithFile: (NSString *)filename;


-(BOOL)containsID: (unsigned)rid type: (unsigned)type;
-(ResRefRec)resourceForID: (unsigned)rid type: (unsigned)type;
-(NSData *)dataForID: (unsigned)rid type: (unsigned)type;
-(NSString *)stringForID: (unsigned)rid type: (unsigned)type;


@end

@interface IIgsResource (Index)

-(unsigned)countResources;
-(unsigned)countTypes;

-(unsigned)typeAtIndex: (unsigned)index;
-(ResRefRec)resourceAtIndex: (unsigned)index;

-(NSData *)dataAtIndex: (unsigned)index;
-(NSString *)stringAtIndex: (unsigned)index;

@end

@interface IIgsResource (Name)

-(unsigned)resourceIDForName: (NSString *)name type: (unsigned)type;

-(BOOL)containsName: (NSString *)name type: (unsigned)type;

-(ResRefRec)resourceForName: (NSString *)name type: (unsigned)type;

-(NSData *)dataForName: (NSString *)name type: (unsigned)type;
-(NSString *)stringForName: (NSString *)name type:(unsigned)type;


@end
