/*
 *  ResourceManager.cpp
 *  IIgsResource
 *
 *  Created by Kelvin Sherlock on 9/12/2009.
 *  Copyright 2009 University of Florida Physicians. All rights reserved.
 *
 */

#include "ResourceManager.h"



#include <cstdlib>
#include <cstring>

#include <algorithm>

using namespace IIgs;




    

    
    const static ResID resNameOffset        = 0x10000;
    const static uint16_t resNameVersion    = 0x0001;
    
    
    
    const static ResourceRecord InvalidRecord = { 0, 0, 0, 0, 0, -1 };
        
    typedef std::vector<ResourceRecord>::iterator ResourceRecordIter;
    typedef std::vector<ResType>::iterator ResTypeIter;
    
    
    static bool Sort(const ResourceRecord& a, const ResourceRecord& b)
    {
        if (a.resType == b.resType) return a.resID < b.resID;
        
        return a.resType < b.resType;
    }
    
    static inline unsigned read16(const uint8_t *x)
    {
        return x[0] | (x[1] << 8);
    }
    
    static inline unsigned read32(const uint8_t *x)
    {
        return x[0] | (x[1] << 8) | (x[2] << 16) | (x[3] << 24);
    }

    
    
    ResourceManager::ResourceManager(const uint8_t *data, unsigned length, unsigned options)
    {
        if (options & rmCopy)
        {
            options |= rmDelete;
            uint8_t *tmp = new uint8_t[length];
            std::memcpy(tmp, data, length);
            data = tmp;
        }
        
        
        if (options & rmDelete)
            options &=  ~rmFree;
        
        
        _data = data;
        _length = length;
        _options = options;
        _error = 0;
        
        open();
    }
    
    
    ResourceManager::~ResourceManager()
    {
        if (_options & rmFree) { if (_data) std::free((void *)_data); }
        if (_options & rmDelete) delete[] _data;
    }

    
    void ResourceManager::setError(unsigned error)
    {
        _error = error;
        if (error && _options & rmThrow)
        {
            throw error;
        }
    }
    
    // parse the data...
    void ResourceManager::open()
    {
        /*
         * File Header:
         * 0 uint32_t rFileVersion (0)
         * 4 uint32_t rFileToMap
         * 8 uint32_t rFileMapSize
         */
        
        if (_data == NULL || _length < 16)
        {
            setError(resBadFormat);
            return;
        }

        
        unsigned rFileVersion = read32(_data);
        unsigned rFileToMap = read32(_data + 4);
        unsigned rFileMapSize = read32(_data + 8);
        
        if (rFileVersion != 0 || _length < rFileToMap + rFileMapSize || rFileMapSize < 30)
        {
            setError(resBadFormat);
            return;
        }

        
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

        const uint8_t *cp = _data + rFileToMap;
        
        //verify numbers are consistent.
        if ((rFileToMap != read32(cp + 6)) || (rFileMapSize != read32(cp + 10))) 
        {
            setError(resBadFormat);
            return;
        }
        
        unsigned mapIndex = read16(cp + 14);
        unsigned mapIndexSize = read32(cp + 20);
        unsigned mapIndexUsed = read32(cp + 24);        
        
        // verify enough space for map index.
        if (_length < rFileToMap + mapIndex + mapIndexSize * 20)
        {
            setError(resBadFormat);
            return;
        }
        
        // build the indexes....
        
        _resources.reserve(mapIndexUsed);
        

        cp = _data + rFileToMap + mapIndex;
        
        for (unsigned i = 0; i < mapIndexUsed; ++i)
        {
            ResourceRecord r;
            
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
            
            if (r.resType == 0 || r.resID == 0)
            {
                setError(resInvalidTypeOrID);
                return;
            }
            
            if (r.resOffset + r.resSize > _length)
            {
                setError(resBadFormat);
                return;                
            }
            
            _resources.push_back(r);
        }    
        
        // sort by type and resource ID.
        std::sort(_resources.begin(), _resources.end(), Sort);
        
        // build the type list
        
        if (!_resources.empty())
        {
            ResType t = 0;
            for (unsigned i = 0; i < _resources.size(); ++i)
            {
                ResourceRecord& r = _resources[i];
                r.resIndex = i;
                if (r.resType != t)
                {
                    _types.push_back(t = r.resType);
                }
            }
            
        }

    }    

    ResType ResourceManager::getIndexedType(unsigned index)
    {
        if (index >= countTypes())
        {
            setError(resIndexRange); 
            return 0;
        }
        
        _error = 0;
        return _types[index];
    }
    
    
    ResID ResourceManager::getIndexedResource(ResType resType, unsigned index)
    {
        if (resType == 0) 
        {
            setError(resInvalidTypeOrID); 
            return 0;
        }        
        
        for (ResourceRecordIter iter = _resources.begin(); iter != _resources.end(); ++iter)
        {
            if (iter->resType == resType)
            {
                if (index == 0)
                {
                    _error = 0;
                    return iter->resID;
                }
                --index;
                continue;
            }
            if (iter->resType > resType) break;
        }
        
        setError(resIndexRange);
        return 0;
    }
    
    
    ResourceRecord ResourceManager::getIndexedResourceRecord(unsigned index)
    {

        if (index >= countResources())
        {
            setError(resIndexRange); 
            return InvalidRecord;
        }
        
        _error = 0;
        return _resources[index];        
    }


    std::pair<ResourceRecord, const uint8_t *> ResourceManager::getIndexedResource(unsigned index)
    {        
        ResourceRecord r = getIndexedResourceRecord(index);
        
        return _error 
            ? std::make_pair(InvalidRecord, (const uint8_t *)NULL) 
            : std::make_pair(r, _data + r.resOffset);
    }


    
    
    ResourceRecord ResourceManager::getResourceRecord(ResType resType, ResID resID)
    {
        if (resType == 0 || resID == 0) 
        {
            setError(resInvalidTypeOrID); 
            return InvalidRecord;
        }        
        
        for (ResourceRecordIter iter = _resources.begin(); iter != _resources.end(); ++iter)
        {
            if (iter->resType == resType && iter->resID == resID)
            {
                _error = 0;
                return *iter;
            }
            if (iter->resType > resType) break;
        }
    
        setError(resNotFound);
        return InvalidRecord;
    }
    
    ResAttr ResourceManager::getResourceAttr(ResType resType, ResID resID) 
    {    
        ResourceRecord r = getResourceRecord(resType, resID);
        return r.resAttr;
    }
    
    unsigned ResourceManager::getResourceSize(ResType resType, ResID resID)
    {
        ResourceRecord r = getResourceRecord(resType, resID);
        return r.resSize;
    }
    
    std::pair<const uint8_t *, unsigned> ResourceManager::loadResource(ResType resType, ResID resID)
    {
        ResourceRecord r = getResourceRecord(resType, resID);
        return _error ? std::make_pair((const uint8_t *)NULL, 0u) : std::make_pair(_data + r.resOffset, r.resSize);
    }

    
    
#pragma mark NamedResources
    
    ResID ResourceManager::findNamedResource(ResType resType, const std::string& name)
    {
        return findNamedResource(resType, name.c_str(), name.length());
    }
    
    ResID ResourceManager::findNamedResource(ResType resType, const char * name)
    {
        return findNamedResource(resType, name, name ? std::strlen(name) : 0);
    }
    

    
    std::pair<const uint8_t *, unsigned> ResourceManager::loadNamedResource(ResType resType, const std::string& name)
    {
        return loadNamedResource(resType, name.c_str(), name.length());
    }
    
    std::pair<const uint8_t *, unsigned> ResourceManager::loadNamedResource(ResType resType, const char* name)
    {
        return loadNamedResource(resType, name, name ? std::strlen(name) : 0);        
    }

    
    std::string ResourceManager::getResourceName(ResType resType, ResID resID)
    {
        
        if (resType == 0 || resID == 0) 
        {
            setError(resInvalidTypeOrID); 
            return "";
        }
        
        
        std::pair<const uint8_t *, unsigned> nameRec = loadResource(rResName, resNameOffset + resType);
        
        if (_error)
        {
            return "";
        }
        

        
        /*
         * uint16_t version
         * uint32_t count
         * [0..count-1] { uint32_t id, pstring name }
         */
        
        unsigned size = nameRec.second;
        const uint8_t *data = nameRec.first;
        unsigned offset = 0;
        unsigned count;
        
        
        if (size < 6)
        {
            setError(resNameNotFound);
            return 0;
        }
        
        if (resNameVersion != read16(data))
        {
            setError(resBadNameVers);
            return 0;
        }
        
        offset += 2;
        
        count = read32(data + offset);
        offset += 4;
        
        for (unsigned i = 0; i < count; ++i)
        {
            // must have space for uint32_t + length byte.
            if (offset + 5 >= size) break;
            
            ResID rID = read32(data + offset);
            offset += 4;
            
            unsigned l = data[offset];
            offset += 1;
            if (offset + l >= size) break;

            if (resID == rID)
            {
                _error = 0;
                return std::string((const char *)(data + offset), l);
            }
            
            
        }
     
        
        setError(resNameNotFound);
        return "";
    }
    
    ResID ResourceManager::findNamedResource(ResType resType, const char *name, unsigned nameLength)
    {
        if (resType == 0) 
        {
            setError(resInvalidTypeOrID); 
            return 0;
        }
        
        if (name == NULL || nameLength == 0)
        {
            setError(resNameNotFound);
            return 0;            
        }
        

        std::pair<const uint8_t *, unsigned> nameRec = loadResource(rResName, resNameOffset + resType);
        if (_error)
        {
            return 0;
        }
        
        /*
         * uint16_t version
         * uint32_t count
         * [0..count-1] { uint32_t id, pstring name }
         */
        
        unsigned size = nameRec.second;
        const uint8_t *data = nameRec.first;
        unsigned offset = 0;
        unsigned count;
        
        
        if (size < 6)
        {
            setError(resNameNotFound);
            return 0;
        }
        
        if (resNameVersion != read16(data))
        {
            setError(resBadNameVers);
            return 0;
        }
        
        offset += 2;
        
        count = read32(data + offset);
        offset += 4;
        
        for (unsigned i = 0; i < count; ++i)
        {
            // must have space for uint32_t + length byte.
            if (offset + 5 >= size) break;
            
            ResID rID = read32(data + offset);
            offset += 4;
            
            unsigned l = data[offset];
            offset += 1;
            if (offset + l >= size) break;
            
            if (l != nameLength)
            {
                offset += l;
                continue;
            }

            if (std::strcmp((const char *)data + offset, name) == 0)
            {
                _error = 0;
                return rID;
            }
        }
        
        setError(resNameNotFound);
        return 0;
    }
    
    std::pair<const uint8_t *, unsigned> ResourceManager::loadNamedResource(ResType resType, const char* name, unsigned nameLength)
    {
        ResID resID = findNamedResource(resType, name, nameLength);
        if (_error) return std::make_pair((const uint8_t *)NULL, 0u);
        
        ResourceRecord r = getResourceRecord(resType, resID);
        if (_error) return std::make_pair((const uint8_t *)NULL, 0u);
        
        return std::make_pair(_data + r.resOffset, r.resSize);
    }
    
    
