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

To build list-of-lists-of-strings from xml string:

    var nkit = require('nkit4nodejs');
    var mapping = ["/person", ["/phone", "string"]];
    var gen = new nkit.Xml2VarBuilder(mapping);
    gen.feed(xmlString); // can be more than one call to feed(xmlChunk) method
    var target = gen.end();
    console.log(JSON.stringify(target, null, '  ')); // prints list of lists of string

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
    
To build list-of-objects-with-lists from xml string:

    var nkit = require('nkit4nodejs');
    var mapping = ["/person",
        {
            "/birthday": "datetime|1970-01-01|%Y-%m-%d",
            "/phone -> phones": ["/", "string"],
            "/address -> cities": ["/city", "string"],
                // same as "/address/city -> cities": ["/", "string"]
            "/married/@firstTime -> isMerriedFirstTime": "boolean"
        }
    ];
    var gen = new nkit.Xml2VarBuilder(mapping);
    gen.feed(xmlString); // can be more than one call to feed(xmlChunk) method
    var target = gen.end();
    console.log(target); // prints list of objects with lists
    
Result:

    [ { cities: [ 'New York', 'Boston' ],
        birthday: Sat Nov 28 1970 00:00:00 GMT+0400 (MSK),
        isMerriedFirstTime: false,
        phones: [ '+122233344550', '+122233344551' ] },
      { cities: [ 'Moscow', 'Tula' ],
        birthday: Wed Jul 16 1969 00:00:00 GMT+0400 (MSK),
        isMerriedFirstTime: true,
        phones: [ '+122233344553', '+122233344554' ] } ]
        
To build object from xml string (last 'person' element will be used):

    var nkit = require('nkit4nodejs');
    var mapping = {
        "/person/name -> lastPersonName": "string",
        "/person/married/@firstTime -> lastPersonIsMarriedFirstTime": "boolean"
    };
    var gen = new nkit.Xml2VarBuilder(mapping);
    gen.feed(xmlString); // can be more than one call to feed(xmlChunk) method
    var target = gen.end();
    console.log(JSON.stringify(target, null, '  '));

Result:

    {
      "lastPersonIsMarriedFirstTime": true,
      "lastPersonName": "Boris"
    }

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

# Author

Boris T. Darchiev (boris.darchiev at gmail.com)

On github: https://github.com/eye3
