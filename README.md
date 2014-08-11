[![Build Status](https://travis-ci.org/eye3/nkit4nodejs.svg?branch=master)](https://travis-ci.org/eye3/nkit4nodejs)

# Introduction

nkit4nodejs - is a [nkit](https://github.com/eye3/nkit.git) C++ library port to Node.js server.

Currently, only xml-to-json-conversion functionality is exported to Node.js

# Installation

## On Linux & Mac OS

    npm install nkit4nodejs
    
## On Windows

Library compiles on MSVS Express version >= 2012:

    set GYP_MSVS_VERSION=2012 
    npm install nkit4nodejs

or

    set GYP_MSVS_VERSION=2013 
    npm install nkit4nodejs

    
# Usage

Suppose, we have this xml string:

    <?xml version="1.0"?>
    <any_name>
        <person>
            <name>Jack</name>
            <phone>+122233344550</phone>
            <phone>+122233344551</phone>
            <age>33</age>
            <married firstTime="No">Yes</married>
            <birthday>1980-02-28</birthday>
            <address>
                <city>New York</city>
                <street>Park Ave</street>
                <buildingNo>1</buildingNo>
                <flatNo>1</flatNo>
            </address>
            <address>
                <city>Boston</city>
                <street>Centre St</street>
                <buildingNo>33</buildingNo>
                <flatNo>24</flatNo>
            </address>
        </person>
        <person>
            <name>Boris</name>
            <phone>+122233344553</phone>
            <phone>+122233344554</phone>
            <age>34</age>
            <married firstTime="Yes">Yes</married>
            <birthday>1979-05-16</birthday>
            <address>
                <city>Moscow</city>
                <street>Kahovka</street>
                <buildingNo>1</buildingNo>
                <flatNo>2</flatNo>
            </address>
            <address>
                <city>Tula</city>
                <street>Lenina</street>
                <buildingNo>3</buildingNo>
                <flatNo>78</flatNo>
            </address>
        </person>
    </any_name>

To create JavaScript object or list of vars from this xml string, we can use 
following scripts.

### To build list-of-strings from xml string:

    var nkit = require('nkit4nodejs');
    
    // Here mapping is list, described by '/path/to/element' and list-item-description.
    // List item here is a 'string' scalar.
    // Scalar definition contains type name and optional default value.
    var mapping = ["/person/phone", "string"];
    //var mapping = ["/person/phone", "string|optionalDefaultValue"];
    
    var builder = new nkit.Xml2VarBuilder(mapping);
    builder.feed(xmlString); // can be more than one call to feed(xmlChunk) method
    var target = builder.end();
    console.log(JSON.stringify(target, null, '  '));

Result:

    [
      "+122233344550",
      "+122233344551",
      "+122233344553",
      "+122233344554"
    ]
    
### To build simple object from xml string (last 'person' xml element will be used):

    var nkit = require('nkit4nodejs');

    // Here mapping is object, described by set of mappings, each containing 
    // key definition and scalar definition.
    // Keys are described by "/path/to/element -> optionalKeyName".
    // If optionalKeyName doesn't provided, then last element 
    // name in '/path/to/element' will be used for key name.
    // Scalar definition may have optional "...|defaultValue"
    var mapping = {
        "/person/name -> lastPersonName": "string|Captain Nemo",
        "/person/married/@firstTime -> lastPersonIsMarriedFirstTime":
            "boolean|true"
    };
    
    var builder = new nkit.Xml2VarBuilder(mapping);
    builder.feed(xmlString); // can be more than one call to feed(xmlChunk) method
    var target = builder.end();
    console.log(JSON.stringify(target, null, '  '));

Result:

    {
      "lastPersonIsMarriedFirstTime": true,
      "lastPersonName": "Boris"
    }

### To build list-of-lists-of-strings from xml string:

    var nkit = require('nkit4nodejs');
    
    // Here mapping is list, described by '/path/to/element' and list-item-description.
    // List item is described as 'list' sub-mapping, described by sub-path and
    // 'string' scalar definition
    var mapping = ["/person", ["/phone", "string"]];
    
    var builder = new nkit.Xml2VarBuilder(mapping);
    builder.feed(xmlString); // can be more than one call to feed(xmlChunk) method
    var target = builder.end();
    console.log(JSON.stringify(target, null, '  '));

Result:

    [
      [
        "+122233344550",
        "+122233344551"
      ],
      [
        "+122233344553",
        "+122233344554"
      ]
    ]
    
### To build list-of-objects-with-lists from xml string:

    var nkit = require('nkit4nodejs');
    
    // Here mapping is list, described by '/path/to/element' and list-item-description.
    // List item is described as 'object' sub-mapping.
    // This 'object' sub-mapping described by set of mappings, each containing 
    // key definition and sub-mapping or scalar.
    // Keys are described by "/sub/path -> optionalKeyName".
    // If optionalKeyName doesn't provided, then last element name in '/sub/path' 
    // will be used for key name
    // Scalar definition may have optional "...|defaultValue"
    // 'datetime' scalar definition MUST contain default value and formatting string
    var mapping = ["/person",
        {
            "/birthday": "datetime|1970-01-01|%Y-%m-%d",
            "/phone -> phones": ["/", "string"],
            "/address -> cities": ["/city", "string"],
                // same as "/address/city -> cities": ["/", "string"]
            "/married/@firstTime -> isMerriedFirstTime": "boolean"
        }
    ];
    
    var builder = new nkit.Xml2VarBuilder(mapping);
    builder.feed(xmlString); // can be more than one call to feed(xmlChunk) method
    var target = builder.end();
    console.log(target);
    
Result:

    [ { cities: [ 'New York', 'Boston' ],
        birthday: Sat Nov 28 1970 00:00:00 GMT+0400 (MSK),
        isMerriedFirstTime: false,
        phones: [ '+122233344550', '+122233344551' ] },
      { cities: [ 'Moscow', 'Tula' ],
        birthday: Wed Jul 16 1969 00:00:00 GMT+0400 (MSK),
        isMerriedFirstTime: true,
        phones: [ '+122233344553', '+122233344554' ] } ]
        
### Notes

Possible scalar types:

    - string
    - integer
    - number    // with floating point
    - datetime  // on Windows - without localization support yet
    - boolean
    
Scalar types can be followed by '|' sign and default value

**datetime** type MUST be followed by '|' sign, default value,
another '|' sign and format string. See 
[man strptime](http://linux.die.net/man/3/strptime) for datetime formatting
syntax. Default value of datetime must correspond to format string.

Path in mapping specifications are very simple XPath now. Only

    /element/with/optional/@attribute
    
paths are supported.
    
JavaScript object keys get their names from the last element name in the path.
If you want to change key names, use this notation:

    "/path/to/element -> newKeyName": ...
    "/path/to/element/@attribute -> newKeyName": ...

# TODO

    - /path/with/*/sign/in/any/place
    
    - More then one 'mapping' parameters for nkit.Xml2VarBuilder(...) constructor to
      create more then one JavaScript data structures from one xml string:
      
          var mapping1 = ...;
          var mapping2 = ...;
          var builder = nkit.Xml2VarBuilder(mapping1, mapping2);
          builder.feed(xmlString);
          var target_list = builder.end();
          var target1 = target_list[0];
          var target2 = target_list[1];

# Author

Boris T. Darchiev (boris.darchiev@gmail.com)

On github: https://github.com/eye3
