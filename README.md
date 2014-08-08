# Introduction

nkit4nodejs - is a nkit C++ library port to Node.js server.

# Installation

## On Linux

    npm install nkit4nodejs
    
## On Windows

    set GYP_MSVS_VERSION=2012
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
following script:

    var nkit = require('nkit4nodejs');
    
    // build list-of-lists-of-strings from xml string
    var builder = new nkit.Xml2VarBuilder(
        ["/person", ["/phone", "string|defaultString"]]);
    builder.feed(xmlString);
    var list = builder.end();
    console.log(list);
    
    // build list-of-objects-with-list from xml string
    builder = new nkit.Xml2VarBuilder(
        ["/person",
            {
                "/birthday": "datetime|1970-01-01|%Y-%m-%d",
                "/phone -> phones": ["/", "string|defaultString"],
                "/married/@firstTime -> isMerriedFirstTime":
                    "boolean"
            }
        ]);
    builder.feed(xmlString); // can be more than one call to
                             // feed(xmlChunk) method
    list = builder.end();
    console.log(list);

Result:

    [ [ '+122233344550', '+122233344551' ],
      [ '+122233344553', '+122233344554' ] ]
      
    [ { birthday: Fri Nov 28 1980 00:00:00 GMT+0400 (MSK),
        isMerriedFirstTime: false,
        phones: [ '+122233344550', '+122233344551' ] },
      { birthday: Mon Jul 16 1979 00:00:00 GMT+0400 (MSK),
        isMerriedFirstTime: true,
        phones: [ '+122233344553', '+122233344554' ] } ]

Possible scalar types:

    - string
    - integer
    - number    //with floating point
    - datetime  // Supported on all platforms except MS Windows,
                // because strptime() function is not supported on this OS.
                // I'm working on it:)
    - boolean
    
Scalar types can be followed by '|' sign and default value

**datetime** type MUST be followed by '|' sign, default value,
another '|' sign and format string. See 
[man strptime](http://linux.die.net/man/3/strptime) for datetime formatting
syntax. Default value of datetime must correspond to format string.

# Author

Boris T. Darchiev (boris.darchiev at gmail.com)

On github: https://github.com/eye3
