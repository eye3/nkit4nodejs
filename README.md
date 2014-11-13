[![Build Status](https://travis-ci.org/eye3/nkit4nodejs.svg?branch=master)](https://travis-ci.org/eye3/nkit4nodejs)







<!-- toc -->

* [Introduction](#introduction)
* [Installation](#installation)
  * [Requirements](#requirements)
  * [On Linux and Mac OS](#on-linux-and-mac-os)
  * [On Windows](#on-windows)
* [XML to JavaScript data conversion](#xml-to-javascript-data-conversion)
  * [Getting started](#getting-started)
  * [Building simple object from xml string (last 'person' xml element will be used)](#building-simple-object-from-xml-string-last-person-xml-element-will-be-used)
  * [Building list-of-objects from xml string](#building-list-of-objects-from-xml-string)
  * [Building list-of-objects-with-lists from xml string](#building-list-of-objects-with-lists-from-xml-string)
  * [Creating keys in object for non-existent xml elements](#creating-keys-in-object-for-non-existent-xml-elements)
  * [Building data structures from big XML source, reading it chunk by chunk](#building-data-structures-from-big-xml-source-reading-it-chunk-by-chunk)
  * [If you want some JSON](#if-you-want-some-json)
  * [Options](#options)
  * [Notes](#notes)
* [JavaScript data to XML conversion](#javascript-data-to-xml-conversion)
  * [Quick start](#quick-start)
  * [Options for xml2var](#options-for-xml2var)
* [Change log](#change-log)
* [Author](#author)

<!-- toc stop -->





# Introduction

nkit4nodejs - is a [nkit](https://github.com/eye3/nkit.git) C++ library port to 
Node.js server.

With nkit4nodejs module you can convert XML string to JavaScript data and vise versa.

**With XML-to-JavaScript-data possibilities you can:**

- Create JavaScript data structures, which are different from the structure 
  of XML source.
  
- Create multiple JavaScript structures from one XML source.

- Explicitly identify those elements and attributes in XML source that you
  want to use for building JavaScript data structures.
  Thus, it's possible to filter out unnecessary XML-data.
  
- Explicitly define Javascript type of scalar (primitive) data, fetched from XML source.
  Integers, numbers, strings, datetimes and booleans are supported.
  
- Control progress of chunked download and parsing of big XML string

- With extra options you can tune some aspects of conversion:
	- trim strings
	- explicitly define white space characters for trim option

Conversion is carried out using SAX parser Expat, so it's fast and uses less 
memory when parsing huge XML files.

This module faster then any other xml-to-JavaScript module, written in pure
JavaScript. For example, nkit4nodejs is about 10 times faster than popular 
[xml2js](https://www.npmjs.org/package/xml2js) module on parsing 
20Mb XML file (see test/compare.js for comparison code).

Module supports not only native Expat XML encodings, but also many others
(see /deps/nkit/src/encoding/langs.inc)

**With JavaScript-Data-to-XML possibilities you can:**

- Create xml string with the same structure as JavaScript data
- Define root element name of result xml string
- Define item element name for arrays
- Define encoding of result xml string
- Pretty print with custom indentation and newline characters
- Define special object key name for attributes
- Define special object key name for text
- Define which element of result xml string must contain CDATA section
- Define precision for float numbers
- Define format for Date objects
- Define representation for *true* and *false* values

# Installation

## Requirements

nkit4nodejs module supports node.js v0.8 - v0.11

nkit4nodejs module must be compiled, so you have to install "build essentials"
(if not yet):

- for debian compatible linux (Debian, Ubuntu, etc.):

	sudo apt-get install build-essential

- for red hat compatible linux (RHEL, SUSE, CentOS, etc.):

	sudo yum groupinstall "Development Tools"

- for Windows:
	
	Install MS Visual Studio 2012 or higher	([MSVS Express](http://www.visualstudio.com/en-us/products/visual-studio-express-vs.aspx) is ok)
	
- in Mac OS use XCode & brew

## On Linux and Mac OS

    npm install nkit4nodejs
    
## On Windows

Library compiles on MSVS version >= 2012:

    set GYP_MSVS_VERSION=2012 
    npm install nkit4nodejs

or

    set GYP_MSVS_VERSION=2013 
    npm install nkit4nodejs

    
# XML to JavaScript data conversion

## Getting started

Suppose, we have this xml string:

```xml
<?xml version="1.0"?>
<any_name>
    <person>
        <phone>+122233344550</phone>
        <name>Jack</name>
        <phone>+122233344551</phone>
        <age>33</age>
        <married firstTime="No">Yes</married>
        <birthday>Wed, 28 Mar 1979 12:13:14 +0300</birthday>
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
        <phone>+122233344553</phone>
        <name>Boris</name>
        <phone>+122233344554</phone>
        <age>34</age>
        <married firstTime="Yes">Yes</married>
        <birthday>Mon, 31 Aug 1970 02:03:04 +0300</birthday>
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
```

With this script we cat generate two data structures:

```JavaScript
var nkit = require('nkit4nodejs');

mappings = {
    "list_of_strings": ["/person/phone", "string"],
    "list_of_lists_of_strings": ["/person", ["/phone", "string"]]
}

var builder = new nkit.Xml2VarBuilder(mappings);
builder.feed(xmlString);
var result = builder.end();

list_of_strings = result["list_of_strings"]
list_of_lists_of_strings = result["list_of_lists_of_strings"]
```

Value of list_of_strings:

```json
[
  "+122233344550",
  "+122233344551",
  "+122233344553",
  "+122233344554"
]
```
    
Value of list_of_lists_of_strings:

```json
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
```

Let's consider script above.

First of all, we importing nkit4nodejs. It contains class Xml2VarBuilder,
which is responsible for xml-to-JavaScript-structures conversion.

```javascript
var nkit = require('nkit4nodejs');
```

Xml2VarBuilder class uses 'mappings' structure, i.e. some directives about how to perform
conversion. Mappings can be written in JSON string (or they can be JSON-compatible
JavaScript structures, as in our example), they describe conversion process and
final structures of JavaScript data. Our example contains two mappings:
'list_of_strings' and 'list_of_lists_of_strings':

```javascript
mappings = {
    "list_of_strings": ["/person/phone", "string"],
    "list_of_lists_of_strings": ["/person", ["/phone", "string"]]
};
```

This means that after conversion we expect to get two data structures:
list of all phones of all persons, and list of phone lists for each person.

First mapping - ["/person/phone", "string"]. It is enclosed in [] brackets.
This means that we expect to get JavaScript list. This type of mapping is called
'list-mapping'.(Braces - {} - means that we want to get JavaScript objects.
Not in this example - see below).
First item of list-mapping defines the XPath where we want to find
data. Second item defines a sub-mapping, which in our case is a scalar-submapping.
Scalar-submapping contains information about type of data we want to get
('string' in our case). 
During conversion nkit4nodejs module will find all elements at path "/person/phone",
convert their values to JavaScript string and put them into JavaScript list.

Second mapping - ["/person", ["/phone", "string"]] is another list-mapping, but
first item points to "/person" XPath, and second item is list-submapping.
List-submapping also contains two elements: sub-xpath and another submapping
('string' scalar-sabmapping in our case). Sub-xpath MUST be continuation of
parent mapping xpath.
During conversion nkit4nodejs module will find all "person" elements and for
each "person" element it will find all "phone" sub-elements, convert their
values to JavaScript string and put them into JavaScript list, which in turn
will be placed to main list.

Each mapping have to be placed in "mappings" structure with some user defined name. This name will be
used in the future to get actual data from result. In our case these names are:
'list_of_strings' and 'list_of_lists_of_strings'.

Now we create builder object:

```javascript
var builder = new nkit.Xml2VarBuilder(mapping);
```

and feed our xml string to it:

```javascript
builder.feed(xmlString);
```

If we receiving xml by chunks, then it is possible to call builder.feed() method
many times (one time for each chunk in order they received).

After feeding all chunks to builder we call end() method to indicate that xml
has been completely received. Also, builder.end() method returns all data
structures at once (to "result" variable in our case):

```javascript
result = builder.end();
```

Now we can get our structures by their mapping names:

```JavaScript
list_of_strings = result["list_of_strings"]
list_of_lists_of_strings = result["list_of_lists_of_strings"]
```

## Building simple object from xml string (last 'person' xml element will be used)

```javascript
var nkit = require('nkit4nodejs');

mapping = {   // <- opening brace for object-mapping
    
        "/person/name -> lastPersonName": "string|Captain Nemo",
        "/person/married/@firstTime -> lastPersonIsMarriedFirstTime":
            "boolean|True",
        "/person/age": "integer"
    
    }   // <- closing brace of object-mapping

mappings = {"last_person": mappings}

var builder = new nkit.Xml2VarBuilder(mappings);
builder.feed(xmlString);
var result = builder.end();

last_person = result["last_person"]
```

Value of last_person:

```json
{
  "name": "Boris", 
  "is_married_first_time": true,
  "age": 34
}
```

Now we use object-mapping (look at the {} braces). Object mapping consists of
mapping items. In our case there is three mapping items:

1. "/person/name": "string|Captain Nemo"
2. "/person/married/@firstTime -> is_married_first_time": "boolean|True"
3. "/person/age": "integer"

Each mapping item consists of object-key-definition and sub-mapping.

Object-key-definitions are described by "xpath" or "xpath -> optional_key_name".
If no optional_key_name has been provided, then last element name in XPath will
be used for key name ("name" in mapping item #1 and "age" in mapping item #3).
Thus, our result object will contain three items with "name",
"is_married_first_time" and "age" keys.

Value for each mapping item will be constructed from data at provided XPath
according to their sub-mappings. In our example, all sub-mappings are scalars.
Note, that it is possible to use "delault values" by putting them in
scalar-submapping after type definition and "|" delimiter (
"Captain Nemo" in mapping item #1 and "True" in mapping item #2).
Default values for scalars are working only in object-mappings, not in
list-mappings.


## Building list-of-objects from xml string
 
```javascript
var nkit = require('nkit4nodejs');

mapping = ["/person",
    {
        "/birthday": "datetime|Fri, 22 Aug 2014 13:59:06 +0000|%a, %d %b %Y %H:%M:%S %z",
        "/name": "string"
    }
]

mappings = {"persons": mapping}

var builder = new nkit.Xml2VarBuilder(mappings);
builder.feed(xmlString);
var result = builder.end();

persons = result["persons"]
```

Value of persons:

```json
[
  {
    "birthday": "1970-11-28 00:00:00", 
    "name": "Jack"
  }, 
  {
    "birthday": "1969-07-16 00:00:00", 
    "name": "Boris"
  }
]
```

Now we use list-mapping and object-submapping. Module will:

- find all person elements
- for each person element construct object from their "birthday" and "name"
  sub-elements and 
- put those objects to main list.

Node: datetime scalar-mapping MUST consists of three elements, divided by "|":

    "datetime|default-value|format-string-in-C-strptime()-function-syntax"
    
Default value MUST correspond to format string

## Building list-of-objects-with-lists from xml string

 
```javascript
var nkit = require('nkit4nodejs');

mapping = ["/person",
    {
        "/address -> cities": ["/city", "string"]
        "/photos": ["/*", "string"],
        "/name": "string"
    }
]

mappings = {"persons": mapping}

var builder = new nkit.Xml2VarBuilder(mappings);
builder.feed(xmlString);
var result = builder.end();

persons = result["persons"]
```

Value of persons:

```json
[
  {
    "photos": ["img1","img2","img3"],
    "cities": [
      "New York", 
      "Boston"
    ], 
    "name": "Jack"
  }, 
  {
    "photos": ["img3","img4"],
    "cities": [
      "Moscow", 
      "Tula"
    ], 
    "name": "Boris"
  }
]
```

As you can see, you can include list- or object-mappings in each other.
List-mapping can contain list- or object-submapping and vise-versa.
Also, it is possible to use '*' char in XPath.

## Creating keys in object for non-existent xml elements 

```javascript
var nkit = require('nkit4nodejs');

mapping = ["/person",
    {
        "/photos": ["/*", {
            "/ -> url" : "string",
            "/width": "integer|0", // element "width" doesn't exist in xml, 
                                   // but because of default value in scalar-mapping "integer|0"
                                   // key "width" will be created with this default value
            "/height": "integer|0" // element "height" doesn't exist in xml, 
                                   // but because of default value in scalar-mapping "integer|0"
                                   // key "height" will be created with this default value
        }],
        "/name": "string"
    }
]

mappings = {"persons": mapping}

var builder = new nkit.Xml2VarBuilder(mappings);
builder.feed(xmlString);
var result = builder.end();

persons = result["persons"]
```

Value of persons:

```json
[
  {
    "photos": [
      {
        "url": "img1", 
        "width": 0, 
        "height": 0
      }, 
      {
        "url": "img2", 
        "width": 0, 
        "height": 0
      }, 
      {
        "url": "img3", 
        "width": 0, 
        "height": 0
      }
    ], 
    "name": "Jack"
  }, 
  {
    "photos": [
      {
        "url": "img3", 
        "width": 0, 
        "height": 0
      }, 
      {
        "url": "img4", 
        "width": 0, 
        "height": 0
      }
    ], 
    "name": "Boris"
  }
]
```

## Building data structures from big XML source, reading it chunk by chunk

You can use builder.get(mapping_name) method to get currently constructed data.

```javascript
var fs = require('fs');
var nkit = require('nkit4nodejs');

var mapping = ["/person", ["/phone", "string"]];
var builder = new nkit.Xml2VarBuilder({"any_name": mapping});
var rstream = fs.createReadStream(xmlFile);
rstream
    .on('data', function (chunk) {
        builder.feed(chunk);
        var list = builder.get("any_name"); // get currently constructed list
        console.log(list.length);
        //list.splice(0); // uncomment this to clear list every time
    })
    .on('end', function () {
        var result = builder.end()["any_name"];
        console.log("Items count: %d", result.length);
    });
```

## If you want some JSON

Just wrap the result object in a call to JSON.stringify:

```javascript
var nkit = require('nkit4nodejs');

var builder = new nkit.Xml2VarBuilder({"phones": ["/person/phone", "string"]});
builder.feed(xmlString);
var phones = builder.end()["phones"];

// print json
console.error(JSON.stringify(phones));

// pretty print
console.error(JSON.stringify(phones, null, 2));

```

## Options

With options you can tune some aspects of conversion:
 
```javascript
var nkit = require('nkit4nodejs');

mapping = ["/person",
    {
        "/*": "string"
    }
]

mappings = {"persons": mapping}

options = {
    "trim": true,
    "white_spaces": " \t\n\r"
}

var builder = new nkit.Xml2VarBuilder(options, mappings);
builder.feed(xmlString);
var result = builder.end();

persons = result["persons"]
```

Following options are supported:

- "trim": Trim out whitespaces at the beginning and at ending of strings.
   Boolean. true or false. Default is false.
- "white_spaces": Characters which are must be considered as white spaces.
   String. Default - "\t\n\r ", i.e. tab,
new line, carriage return and space.


## Notes

Possible scalar types:

    - string
    - integer
    - number
    - datetime
    - boolean
    
Scalar types can be followed by '|' sign and default value

**datetime** type MUST be followed by '|' sign, default value,
another '|' sign and format string. See 
[man strptime](http://linux.die.net/man/3/strptime) for datetime formatting
syntax. Default value of datetime MUST correspond to format string.

Path in mapping specifications are very simple XPath now. Only

    /path/to/element
    /path/to/element/with/optional/@attribute
    /paths/to/element/with/*/sign
    /paths/to/element/with/*/sign/with/optional/@attribute

paths are supported.

JavaScript object keys get their names from the last element name in the path.
If you want to change key names, use this notation:

    "/path/to/element -> newKeyName": ...
    "/path/to/element/@attribute -> newKeyName": ...

# JavaScript data to XML conversion

## Quick start

```javascript
var nkit = require('nkit4nodejs');

var data = {
    "$": {"p1": "в1&v2\"'", "p2": "v2"},
    "_": "Hello(Привет) world(мир)",
    "int_число": 1,
    "true": true,
    "false": false,
    "float": 1.123456789,
    "cdata1": "text < > & \" '",
    "cdata2": "%^&*()-=+ < > & \" '",
    "list": [[1, 2], 2, 3],
    "datetime": new Date(1979, 2, 28, 12, 13, 14),
    "dict": {
        "$": {"a1": "V1", "a2": "V2"},
        "int": 1,
        "float": 1.11234567891234,
        "sub_string": "text < > & \" '",
        "list": [[1], 2, 3]
    }
};

var ENC = "UTF-8";

var options = {
    "rootname": "ROOT",
    "itemname": "item",
    "encoding": ENC,
    "xmldec": {
        "version": "1.0",
        "standalone": true
    },
    "pretty": {
        "indent": "  ",
        "newline": "\n"
    },
    "attrkey": "$",
    "textkey": "_",
    "cdata": ["cdata1", "cdata2"],
    "float_precision": 10,
    "date_time_format": "%Y-%m-%d %H:%M:%S",
    "bool_true": "Yes",
    "bool_false": "No"
};

console.log(nkit.var2xml(data, options).toString());
```

Output:

```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<ROOT p1="в1&amp;v2&quot;&apos;" p2="v2">
  <int_число>1</int_число>
  <true>Yes</true>
  <false>No</false>
  <float>1.1234567890</float>
  <cdata1><![CDATA[text < > & " ']]></cdata1>
  <cdata2><![CDATA[%^&*()-=+ < > & " ']]></cdata2>
  <list>
    <item>1</item>
    <item>2</item>
  </list>
  <list>2</list>
  <list>3</list>
  <datetime>1979-03-28 12:13:14</datetime>
  <dict a1="V1" a2="V2">
    <int>1</int>
    <float>1.1123456789</float>
    <sub_string>text &lt; &gt; &amp; &quot; &apos;</sub_string>
    <list>
      <item>1</item>
    </list>
    <list>2</list>
    <list>3</list>
  </dict>
  Hello(Привет) world(мир)
</ROOT>
```

## Options for xml2var

Following options are supported:

- **rootname**: name of root element. If not provided then result XML will be w/o xmldec and root element;
- **itemname**: default element name for JavaScript array items. Default - 'item';
- **encoding**: "UTF-8" or some other encodings (see */deps/nkit/src/encoding/langs.inc* for list of supported encodings). Default - "UTF-8";
- **xmldec**: XML declaration. Default - NO XML declaration. Sub-options:

    - *version*: xml version;
    - *standalone*: true or false;

- **pretty**: pretty XML - with indents and custom newlines. Default - NO pretty print, i.e. print XML in single line. Sub-options:

    - *indent*: any string for indentation;
    - *newline*: any string for line ending;
    
- **attrkey**: any string for Object key name, that holds attributes for element. Default '$';
- **textkey**: any string for Object key name, that holds text for element. Default '_';
- **cdata**: array of key names whose values mast be print to XML string as CDATA. Default - empty array;
- **float_precision**: for float numbers - number of symbols after '.' to be printed. Default - 2;
- **date_time_format**: format string of Date objects.
See [man strftime](http://www.cplusplus.com/reference/ctime/strftime/?kw=strftime) for datetime formatting syntax.
Default "%Y-%m-%d %H:%M:%S";
- **bool_true**: representation for 'true' boolean value. Default '1';
- **bool_false**: representation for 'false' boolean value. Default '0';

If no *rootname* has been provided then *xmldec* will no effect.
If **data** is Object (not Array) then *attrkey* will no effect for root Object.

If **data** is Array then *itemname* will be used as element name for its items.


# Change log

- 2.2:
    - Options changes for nkit4nodejs.var2xml(): standalone 'encoding' option.
      In previous version 'encoding' option was in 'xmldec'
      
- 2.1:
    - nkit4nodejs.var2xml() method for converting JavaScript data to XML
    
- 2.0:
    - Multi-mappings and options for nkit4nodejs.Xml2VarBuilder class
    
- 1.X:
    - nkit4nodejs.Xml2VarBuilder class for converting XML to JavaScript data

# Author

Boris T. Darchiev (boris.darchiev@gmail.com)

On github: https://github.com/eye3

Any feedback or pull request are welcome!