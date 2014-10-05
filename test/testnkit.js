var nkit = require('../index.js');
var deep_equal = require('./deep_equal.js');
var fs = require('fs');

var xmlString = fs.readFileSync(__dirname + "/data/sample.xml");

//------------------------------------------------------------------------------
// build list-of-strings from xml string

// Here mapping is list, described by /path/to/element and list item
// description. List item here is a 'string' scalar.
// Scalar definition contains type name and optional default value.
var mapping = ["/person/phone", "string"];
// var mapping = ["/person/phone", "string|optionalDefaultValue"];

var builder = new nkit.Xml2VarBuilder(mapping);
builder.feed(xmlString); // can be more than one call to feed(xmlChunk) method
var result = builder.end();

var etalon = [ '+122233344550',
               '+122233344551',
               '+122233344553',
               '+122233344554' ];

if (!deep_equal.deepEquals(result, etalon)) {
    console.error(JSON.stringify(result, null, 2));
    console.error(JSON.stringify(etalon, null, 2));
    console.error("Error #1");
    process.exit(1);
}


//------------------------------------------------------------------------------
// build simple object from xml string (last 'person' element will be used)

// Here mapping is object, described by set of mappings, each containing
// key definition and scalar definition.
// Keys are described by "/sub/path -> optionalKeyName".
// If optionalKeyName doesn't provided, then last element name in /sub/path
// will be used for key name.
// Scalar definition may have optional "...|defaultValue"
var mapping = {
    "/person/name -> lastPersonName": "string|Captain Nemo",
    "/person/married/@firstTime -> lastPersonIsMarriedFirstTime":
    	"boolean|true",
    "/person/age": "integer"
};

var builder = new nkit.Xml2VarBuilder(mapping);
builder.feed(xmlString); // can be more than one call to feed(xmlChunk) method
var result = builder.end();

//------------------------------------------------------------------------------
// build list-of-lists-of-strings from xml string

// Here mapping is list, described by /path/to/element and list item
// description. List item is described as 'list' sub-mapping, described 
// by sub-path and'string' scalar definition
var mapping = ["/person", ["/phone", "string"]];

var builder = new nkit.Xml2VarBuilder(mapping);
builder.feed(xmlString); // can be more than one call to feed(xmlChunk) method
var result = builder.end();
//console.log(JSON.stringify(result, null, '  ')); // prints list of lists of strings

var etalon = [ [ '+122233344550', '+122233344551' ],
    [ '+122233344553', '+122233344554' ] ];

if (!deep_equal.deepEquals(result, etalon)) {
    console.error(JSON.stringify(result, null, 2));
    console.error(JSON.stringify(etalon, null, 2));
    console.error("Error #2");
    process.exit(1);
}

//------------------------------------------------------------------------------
// build list-of-objects-with-lists from xml string

// Here mapping is list, described by /path/to/element and list item description.
// List item is described as 'object' sub-mapping.
// This 'object' sub-mapping described by set of mappings, each containing
// key definition and sub-mapping or scalar.
// Keys are described by "/sub/path -> optionalKeyName".
// If optionalKeyName doesn't provided, then last element name in "/sub/path"
// will be used for key name
// Scalar definition may have optional "...|defaultValue"
// 'datetime' scalar definition MUST contain default value and formatting string
var mapping = ["/person",
    {
        "/birthday":
        	"datetime|Fri, 22 Aug 2014 13:59:06 +0000|%a, %d %b %Y %H:%M:%S %z",
        "/phone -> phones": ["/", "string"],
        "/address -> cities": ["/city", "string"],
            // same as "/address/city -> cities": ["/", "string"]
        "/married/@firstTime -> isMerriedFirstTime": "boolean",
        "/photos": ["/*", "string"]
    }
];

var builder = new nkit.Xml2VarBuilder(mapping);
builder.feed(xmlString); // can be more than one call to feed(xmlChunk) method
var result = builder.end();
//console.log(result); // prints list of objects with lists

etalon = [
    {
        birthday: new Date(1979, 2, 28, 12, 13, 14),
        isMerriedFirstTime: false,
        phones: [ '+122233344550', '+122233344551' ],
        cities: [ 'New York', 'Boston' ],
        photos: ["img1","img2","img3"]
    },
    {
        birthday: new Date(1970, 7, 31, 2, 3, 4),
        isMerriedFirstTime: true,
        phones: [ '+122233344553', '+122233344554' ],
        cities: [ 'Moscow', 'Tula' ],
        photos: ["img3","img4"]
    }
];

if (!deep_equal.deepEquals(result, etalon)) {
    console.error(JSON.stringify(result, null, 2));
    console.error(JSON.stringify(etalon, null, 2));
    console.error("Error #3");
    process.exit(1);
}

//------------------------------------------------------------------------------
mapping = ["/person",
    {
        "/*": "string"
    }
];

var builder = new nkit.Xml2VarBuilder(mapping);
builder.feed(xmlString);
var result = builder.end();

etalon = [
  {
    "phone": "+122233344551",
    "name": "Jack",
    "photos": "\n\t\t\t\n\t\t\t\n\t\t\t\n\t\t",
    "age": "33",
    "married": "Yes",
    "birthday": "Wed, 28 Mar 1979 12:13:14 +0300",
    "address": "\n\t\t\t\n\t\t\t\n\t\t\t\n\t\t\t\n\t\t",
    "empty": ""
  },
  {
    "phone": "+122233344554",
    "name": "Boris",
    "photos": "\n\t\t\t\n\t\t\t\n\t\t",
    "age": "34",
    "married": "Yes",
    "birthday": "Mon, 31 Aug 1970 02:03:04 +0300",
    "address": "\n\t\t\t\n\t\t\t\n\t\t\t\n\t\t\t\n\t\t",
    "empty": ""
  }
];

if (!deep_equal.deepEquals(result, etalon)) {
    console.error(JSON.stringify(result, null, 2));
    console.error(JSON.stringify(etalon, null, 2));
    console.error("Error #4");
    process.exit(1);
}

console.log("ok");
process.exit(0);
