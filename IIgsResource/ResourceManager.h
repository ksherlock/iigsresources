/*
 *  ResourceManager.h
 *  IIgsResource
 *
 *  Created by Kelvin Sherlock on 9/12/2009.
 *
 */

#ifndef __PRODOS_RESOURCE_MANAGER_H__
#define __PRODOS_RESOURCE_MANAGER_H__

#include <stdint.h>

#ifdef __cplusplus
#include <vector>
#include <utility>
#include <string>

namespace IIgs {
#endif

    
    /*
     * Resource Types
     */
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

    /*
     * Resource Attributes
     */
    enum {
        // memory manager
        resPage             = 0x0004,       /* page aligned */
        resNoSpec           = 0x0008,       /* may not use special memory */
        resNoCross          = 0x0010,       /* may not cross banks */
        attrPurge1          = 0x0100,       /* purge level 1 */
        attrPurge2          = 0x0200,       /* purge level 2 */
        attrPurge3          = 0x0300,       /* purge level 3 */
        resLocked           = 0x8000,       /* locked */
        resFixed            = 0x4000,       /* not moveable */
        
        // resource manager
        resChanged          = 0x0020,
        resPreLoad          = 0x0040,
        resProtected        = 0x0080,
        resAbsLoad          = 0x0400,
        resConverter        = 0x0800
    };
    
    /*
     * Resource Errors
     */
    
    enum {
        resForkUsed         = 0x1E01,       /* Resource fork not empty */
        resBadFormat        = 0x1E02,       /* Format of resource fork is unknown */
        resNoConverter      = 0x1E03,       /* No converter routine available for resource type */
        resNoCurFile        = 0x1E04,       /* there are no current open resource files */
        resDupID            = 0x1E05,       /* ID is already used */
        resNotFound         = 0x1E06,       /* resource was not found */
        resFileNotFound     = 0x1E07,       /* resource file not found */
        resBadAppID         = 0x1E08,       /* User ID not found, please call ResourceStartup */
        resNoUniqueID       = 0x1E09,       /* a unique ID was not found */
        resIndexRange       = 0x1E0A,       /* Index is out of range */
        resSysIsOpen        = 0x1E0B,       /* System file is already open */
        resHasChanged       = 0x1E0C,       /* Resource marked changed; specified operation not allowed */
        resDiffConverter    = 0x1E0D,       /* Different converter already logged in for this resource type */
        resDiskFull         = 0x1E0E,       /* Volume is full */
        resInvalidShutDown  = 0x1E0F,       /* can't shut down ID 401E */
        resNameNotFound     = 0x1E10,       /* no resource with given name */
        resBadNameVers      = 0x1E11,       /* bad version in rResName resource */
        resDupStartUp       = 0x1E12,       /* already started with this ID */
        resInvalidTypeOrID  = 0x1E13        /* type or ID is 0 */       
    };
    
    
    typedef uint32_t ResID;
    typedef uint16_t ResType;
    typedef uint16_t ResAttr;
    
    
    typedef struct ResourceRecord {
        ResType     resType;
        ResID       resID;
        unsigned    resOffset;
        ResAttr     resAttr;
        unsigned    resSize;
        
        unsigned    resIndex;        
        
#ifdef __cplusplus
        bool isValid() const { return resIndex != -1; }
#endif
    } ResourceRecord;

    

    
    
#ifdef __cplusplus


    enum {
        rmCopy      = 1,        // make a copy of the data
        rmFree      = 2,        // use std::free() on the data
        rmDelete    = 4,        // use delete[] on the data
        rmThrow     = 8         // throw errors?
    };
    
    class ResourceManager {

    public:
        
        ResourceManager(const uint8_t *data, unsigned length, unsigned options = 0);
        ~ResourceManager();
        
        unsigned error() const { return _error; }
        
        
        unsigned countTypes() { _error = 0; return _types.size(); }
        unsigned countResources() { _error = 0; return _resources.size(); }
        
        ResType getIndexedType(unsigned index);
        ResID getIndexedResource(ResType resType, unsigned index);
        ResourceRecord getIndexedResourceRecord(unsigned index);        
        std::pair<ResourceRecord, const uint8_t *> getIndexedResource(unsigned index);

        
        ResourceRecord getResourceRecord(ResType resType, ResID resID);
        ResAttr getResourceAttr(ResType resType, ResID resID);
        unsigned getResourceSize(ResType resType, ResID resID);

        
        std::pair<const uint8_t *, unsigned> loadResource(ResType resType, ResID resID);
        
        ResID findNamedResource(ResType resType, const std::string& name);
        ResID findNamedResource(ResType resType, const char * name);

        std::string getResourceName(ResType resType, ResID resID);

        std::pair<const uint8_t *, unsigned> loadNamedResource(ResType resType, const std::string& name);
        std::pair<const uint8_t *, unsigned> loadNamedResource(ResType resType, const char* name);
        
    private:
        
        void open();
        void setError(unsigned error);
        
        ResID findNamedResource(ResType resType, const char *name, unsigned nameLength);
        std::pair<const uint8_t *, unsigned> loadNamedResource(ResType resType, const char* name, unsigned nameLength);
        
        std::vector<ResType>_types;
        std::vector<ResourceRecord>_resources;
        const uint8_t *_data;
        unsigned _length;
        unsigned _options;
        
        unsigned _error; // mutable?
    };

    
#endif



#ifdef __cplusplus
} // namespace
#endif

#endif