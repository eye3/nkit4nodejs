# Introduction

nkit4nodejs - is a nkit C++ library port to Node.js server.

# Installation and Requirements

This module depends on nkit C++ library, which in-turn depends on:

    - yajl
    - expat
    - boost (verstion >= 1.53)

You can find boost, yajl and expat in your OS packages (DEB or RPM).

To install nkit library, you must type following commands:

    git clone https://github.com/eye3/nkit.git
    cd nkit
    ./bootstrap.sh --prefix=$HOME/env --with-boost=/path/to/boost \
            --boost-version=1.53 \
            --with-yajl=/path/to/yajl --with-vx --with-expat=/path/to/expat \
            --with-pic --release
    make -C Release-build
    make -C Release-build install

You can change prefix ($HOME/env) to whatever you want.

For more information about installing of nkit library, see:

    https://github.com/eye3/nkit

To install nkit4nodejs, type

    export NKIT_ROOT=$HOME/env
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
    var builder = new nkit.Xml2VarBuilder('
        ["/person",
            ["/phone", "string|defaultString"]]
        ');
    builder.feed(xmlString);
    var list = builder.end();
    console.log(list);
    
    // build list-of-objects-with-list from xml string
    builder = new nkit.Xml2VarBuilder('
        ["/person",
            {
                "/birthday": "datetime|1970-01-01|%Y-%m-%d",
                "/phone -> phones": ["/", "string|defaultString"],
                "/married/@firstTime -> isMerriedFirstTime":
                    "string|defaultString"
            }
        ]');
    builder.feed(xmlString); // can be more than one call to
                             // feed(xmlChunk) method
    list = builder.end();
    console.log(list);

Possible scalar types:
    - string
    - integer
    - number (with floating point)
    - datetime
    
Scalar types can be followed by '|' sign and default value

**datetime** type MUST be followed by '|' sign, default value,
another '|' sign and format string. See 
[man strptime](http://linux.die.net/man/3/strptime) for datetime formatting
syntax. Default value of datetime must correspond to format string.

# Author

Boris T. Darchiev (boris.darchiev at gmail.com)

On github: https://github.com/eye3

