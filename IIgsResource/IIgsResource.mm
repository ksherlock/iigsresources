//
//  IIgsResource.mm
//  IIgsResource
//
//  Created by Kelvin Sherlock on 9/9/2009.
//



#include <algorithm>
#include <vector>
#include <memory>
#include <cstring>

#include <sys/xattr.h>

#import "IIgsResource.h"

// attr flags
#define resChanged 0x0020
#define resPreLoad 0x0040
#define resProtected 0x0080
#define resAbsLoad 0x0400
#define resConverter 0x0800

#define resNameOffset 0x10000
#define resNameVersion 0x0001


#define iigs_sizeof_resrefrec 20

static ResRefRec FailRec = { 0, 0, 0, 0, 0, -1 };


static inline unsigned read16(const uint8_t *x)
{
    return x[0] | (x[1] << 8);
}

static inline unsigned read32(const uint8_t *x)
{
    return x[0] | (x[1] << 8) | (x[2] << 16) | (x[3] << 24);
}

static bool Sort(const ResRefRec& a, const ResRefRec& b)
{
    if (a.resType == b.resType) return a.resID < b.resID;
    
    return a.resType < b.resType;
}

static bool IsStringResource(unsigned type)
{
    switch(type)
    {
        case rC1OutputString:
        case rWString:
        case rC1InputString:
        case rPString:
        case rTextBlock:
        case rText:
        case rComment:
        case rCString:
            return YES;
            break;
        default:
            return NO;
            break;
    }
}


static NSString *ResourceToString(ResRefRec r, uint8_t *data)
{

    unsigned length;
    unsigned fudge;
    
    unsigned size = r.resSize;
    unsigned offset = 0;
    
    switch(r.resType)
    {
            // 2 byte word + 2 byte size + text
        case rC1OutputString:
            offset += 2;
            size -= 2;
            
            // 2 byte size prefix followed by text.
        case rWString:
        case rC1InputString:
            fudge = 2;
            length = read16(data + offset);
            break;
            
            // 1 byte size prefix + text
        case rPString:
            fudge = 1;
            length = *data;
            break;
            
            // text
        case rTextBlock:
        case rText:
        case rComment:
            fudge = 0;
            length = size;
            break;
            
            // null-terminated text
        case rCString:
            fudge = 0;
            // sanity check to verify is null terminated
            for (length = 0; data[length] && (length < size); ++length) ;
            break;
            
        default:
            return nil;
    }
    
    
    if (fudge + length > size) return nil;
    return [[[NSString alloc] initWithBytes: data + offset + fudge length: length encoding: NSMacOSRomanStringEncoding] autorelease];
    

}


typedef std::vector<ResRefRec> ResRefVector;
typedef std::vector<ResRefRec>::iterator ResRefRecIter;
typedef std::vector<unsigned> UnsignedVector;


@interface IIgsResource (Private)
-(BOOL)parseResource;
@end


@implementation IIgsResource (Private)

-(BOOL)parseResource
{
    /*
     * File Header:
     * 0 uint32_t rFileVersion (0)
     * 4 uint32_t rFileToMap
     * 8 uint32_t rFileMapSize
     */
    
    if (_size < sizeof(uint32_t) * 4) return NO;
    
    unsigned rFileVersion = read32(_resource);
    unsigned rFileToMap = read32(_resource + 4);
    unsigned rFileMapSize = read32(_resource + 8);
    
    if (rFileVersion != 0) return NO;
    
    if (_size < rFileToMap + rFileMapSize) return NO;
    if (rFileMapSize < 30) return NO;
    
    /*
     * Resource Map Header:
     *
     * 0 uint32_t handle (0)
     * 4 uint16_t map flag
     * 6 uint32_t map offset (should match file header)
     * 10 uint32_t map size (should match file header)
     * 14 uint16_t map to index
     * 16 uint16_t map file num
     * 18 uint16_t map id
     * 20 uint32_t map index size
     * 24 uint32_t map index used
     * 28 uint16_t map free list size
     * 30 uint16_t map free list used
     */
    
    
    const uint8_t *cp = _resource + rFileToMap;
    
    if (rFileToMap != read32(cp + 6)) return NO;
    if (rFileMapSize != read32(cp + 10)) return NO;
    
    unsigned mapIndex = read16(cp + 14);
    unsigned mapIndexSize = read32(cp + 20);
    unsigned mapIndexUsed = read32(cp + 24);
    
    // rFileToMap + mapIndex = index
    
    // verify enough space for map index.
    if (_size < rFileToMap + mapIndex + mapIndexSize * 20) return NO;

    
    // build the index...
    
    std::auto_ptr<ResRefVector> v(new ResRefVector);
    
    v->reserve(mapIndexUsed);

    cp = _resource + rFileToMap + mapIndex;

    for (unsigned i = 0; i < mapIndexUsed; ++i)
    {
        ResRefRec r;
        
        r.resType = read16(cp);
        cp += 2;
        
        r.resID = read32(cp);
        cp += 4;
        
        r.resOffset = read32(cp);
        cp += 4;
        
        r.resAttr = read16(cp);
        cp += 2;
        
        r.resSize = read32(cp);
        
        cp += 4;
        // handle.
        cp += 4;
        
        
        if (r.resOffset + r.resSize > _size) return NO;
        
        v->push_back(r);
        
    }
    
    // sort the vector by type and resource id.
    
    std::sort(v->begin(), v->end(), Sort);
    
    UnsignedVector *types = new UnsignedVector;

    for (unsigned i = 0, tmp = -1; i < v->size(); ++i)
    { 
        ResRefVector &r = *v;
        if (r[i].resType != tmp)
            types->push_back(tmp = r[i].resType);
        r[i].resIndex = i;
    }
    
    
    _map = v.release();
    _types = types;
    
    return YES;
    
}



@end


@implementation IIgsResource





+(id)resourceWithFile:(NSString *)filename
{
    return [[[self alloc] initWithFile: filename] autorelease];
}

-(id)initWithFile:(NSString *)filename
{
    
    if (self = [self init])
    {
        
        const char *path = [filename fileSystemRepresentation];
        ssize_t size = getxattr(path, "prodos.ResourceFork", NULL, 0, 0, 0);
        if (size <= 0)
        {
            [self release];
            return nil;
        }
        
        _resource = new uint8_t[size];
        _size = size;
        
        if (size != getxattr(path, "prodos.ResourceFork", _resource, size, 0, 0))
        {
            [self release];
            return nil;
        }
        
        
        if (![self parseResource])
        {
            [self release];
            return nil;
        }
    }
    return self;
}


-(void)dealloc
{
    delete []_resource;
    delete (ResRefVector *)_map;
    delete (UnsignedVector *)_types;
    
    [super dealloc];
}


-(BOOL)containsID: (unsigned)rid type: (unsigned)type
{
    ResRefRec r = [self resourceForID: rid type: type];
    return (r.resOffset > 0);
}








-(NSData *)dataForID: (unsigned)rid type: (unsigned)type
{
    ResRefRec r = [self resourceForID: rid type: type];
    if (r.resOffset == 0) return nil;
    
    return [NSData dataWithBytes: _resource + r.resOffset length: r.resSize];
}

/*
 * return a resource as an NSString, if we can.
 *
 */



-(NSString *)stringForID: (unsigned)rid type: (unsigned)type
{
    ResRefRec r = [self resourceForID: rid type: type];
    if (r.resOffset == 0) return nil;
    
    return ResourceToString(r, _resource + r.resOffset);
  
}





-(ResRefRec)resourceForID: (unsigned)rid type: (unsigned)type
{
    
    ResRefVector *map = (ResRefVector *)_map;
    
    for (ResRefRecIter iter = map->begin(); iter != map->end(); ++iter)
    {
        if (iter->resType == type && iter->resID == rid) return *iter;
        if (iter->resType > type) break;
    }
    
        
    return FailRec;
}







@end



@implementation IIgsResource (Index)


-(unsigned)countTypes
{
    UnsignedVector *types = (UnsignedVector *)_types;
    
    return types->size();
}

-(unsigned)countResources
{
    ResRefVector *map = (ResRefVector *)_map;
    
    return map->size();
}

-(unsigned)typeAtIndex: (unsigned)index
{
    UnsignedVector *types = (UnsignedVector *)_types;
    
    if (index >= types->size())
    {
        [NSException raise: NSRangeException format: @"index %u beyond size %u", index, types->size() ];
    }
    return (*types)[index];
}

-(ResRefRec)resourceAtIndex: (unsigned)index
{
    ResRefVector *map = (ResRefVector *)_map;
    
    if (index >= map->size())
    {
        [NSException raise: NSRangeException format: @"index %u beyond size %u", index, map->size() ];
    }
    return (*map)[index];    
}

-(NSData *)dataAtIndex: (unsigned)index
{
    ResRefVector *map = (ResRefVector *)_map;
    
    if (index >= map->size())
    {
        [NSException raise: NSRangeException format: @"index %u beyond size %u", index, map->size() ];
    }
    
    ResRefRec r = (*map)[index];
    
    return [NSData dataWithBytes: _resource + r.resOffset length: r.resSize];
    
}


-(NSString *)stringAtIndex: (unsigned)index
{
    ResRefVector *map = (ResRefVector *)_map;
    
    if (index >= map->size())
    {
        [NSException raise: NSRangeException format: @"index %u beyond size %u", index, map->size() ];
    }
    
    ResRefRec r = (*map)[index];
    
    return ResourceToString(r, _resource + r.resOffset);
}


@end


@implementation IIgsResource (Name)


-(unsigned)resourceIDForName: (NSString *)name type: (unsigned)type
{
    // rResname (id is 0x00010000 | type)
    /*
     * uint16_t format
     * uint32_t numEntries
     * { uint32_t id, pstring name }*
     *
     */
    
    
    ResRefRec r = [self resourceForID:  type + resNameOffset type: rResName];
    
    if (r.resOffset == 0) return 0;
    
    
    
    //unsigned size = r.resSize;
    const uint8_t *cp = _resource + r.resOffset;
    const char *nm = [name cStringUsingEncoding: NSMacOSRomanStringEncoding];
    if (!nm) return 0;
    
    unsigned nmLength = std::strlen(nm);
    
    // scan for the name...
    
    if (read16(cp) != resNameVersion) return 0;
    cp += 2;
    unsigned numEntries = read32(cp);
    cp += 4;
    
    // this doesn't check overflow....
    
    for (unsigned i = 0; i < numEntries; ++i)
    {
        unsigned rid = read32(cp);
        cp += 4;
        
        unsigned tmp = *cp++;
        if (tmp == nmLength && strcmp(nm, (const char *)cp))
        {
            return rid;
        }
        cp += tmp;
        
    }
    
    return 0;
    
}





-(BOOL)containsName: (NSString *)name type: (unsigned)type
{
    unsigned rid = [self resourceIDForName: name type: type];
    if (rid == 0) return NO;
    
    return [self containsID: rid type: type];
}



-(NSData *)dataForName: (NSString *)name type: (unsigned)type
{
    unsigned rid = [self resourceIDForName: name type: type];
    if (rid == 0) return nil;
    
    return [self dataForID: rid type: type];
}


-(NSString *)stringForName: (NSString *)name type:(unsigned)type
{
    unsigned rid = [self resourceIDForName: name type: type];
    if (rid == 0) return nil;
    
    return [self stringForID: rid type: type];
}


-(ResRefRec)resourceForName: (NSString *)name type: (unsigned)type
{
    unsigned rid = [self resourceIDForName: name type: type];
    if (rid == 0) return FailRec;
    
    return [self resourceForID: rid type: type];
}


@end
