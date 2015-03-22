
```
+(id)resourceWithFile: (NSString *)filename;
```

```
-(id)initWithFile: (NSString *)filename;
```

Reads the `prodos.ResourceFork` extended attribute (tip: profuse compatible) and -- if present and valid -- returns a IIgsResource object.  Returns nil on error.  This is liable to change at some point to be more flexible.



```
-(unsigned)countTypes;
```

Returns the number of distinct resource types.

```
-(unsigned)typeAtIndex: (unsigned)index;
```

returns the type at a given index.  Out of range errors will generate an NSException.


```
-(unsigned)countResources;
```

Returns the number of resources.

```
-(ResRefRec)resourceAtIndex: (unsigned)index;
```

Returns the ResRefRec (type, id, attribute, size, etc) at a given index.  Out of range errors will generate an NSException.

```
-(NSData *)dataAtIndex: (unsigned)index;
```

Returns the resource data, as an NSData object,  at a given index.  Out of range errors will generate an NSException.


```
-(NSString *)stringAtIndex: (unsigned)index;
```

Returns the resource data, as an NSString object (MacRoman encoded),  at a given index.  Out of range errors will generate an NSException.  If the type (or data) is not string-like (eg, rPString), nil will be returned.

