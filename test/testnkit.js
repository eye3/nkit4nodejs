var nkit = require('../index.js');
var fs = require('fs');

var xmlString = fs.readFileSync(__dirname + "/data/sample.xml");

//------------------------------------------------------------------------------
// build list-of-strings from xml string

// Here mapping is list, described by /path/to/element and list item description.
// List item here is a 'string' scalar.
// Scalar definition contains type name and optional default value.
var mapping = ["/person/phone", "string"];
//var mapping = ["/person/phone", "string|optionalDefaultValue"];

var builder = new nkit.Xml2VarBuilder(mapping);
builder.feed(xmlString); // can be more than one call to feed(xmlChunk) method
var target = builder.end();
console.log(JSON.stringify(target, null, '  ')); // prints list of strings

var etalon = [ '+122233344550', '+122233344551', '+122233344553', '+122233344554' ];

if (JSON.stringify(target) !== JSON.stringify(etalon)) {
    console.error(JSON.stringify(target));
    console.error(JSON.stringify(etalon));
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
    "/person/married/@firstTime -> lastPersonIsMarriedFirstTime": "boolean|true"
};

var builder = new nkit.Xml2VarBuilder(mapping);
builder.feed(xmlString); // can be more than one call to feed(xmlChunk) method
var target = builder.end();
console.log(JSON.stringify(target, null, '  '));

//------------------------------------------------------------------------------
// build list-of-lists-of-strings from xml string

// Here mapping is list, described by /path/to/element and list item description.
// List item is described as 'list' sub-mapping, described by sub-path and
// 'string' scalar definition
var mapping = ["/person", ["/phone", "string"]];

var builder = new nkit.Xml2VarBuilder(mapping);
builder.feed(xmlString); // can be more than one call to feed(xmlChunk) method
var target = builder.end();
console.log(JSON.stringify(target, null, '  ')); // prints list of lists of strings

var etalon = [ [ '+122233344550', '+122233344551' ],
    [ '+122233344553', '+122233344554' ] ];

if (JSON.stringify(target) !== JSON.stringify(etalon)) {
    console.error(JSON.stringify(target));
    console.error(JSON.stringify(etalon));
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
console.log(target); // prints list of objects with lists

etalon = [
    {
        cities: [ 'New York', 'Boston' ],
        birthday: new Date(1970, 10, 28),
        isMerriedFirstTime: false,
        phones: [ '+122233344550', '+122233344551' ] },

    {
        cities: [ 'Moscow', 'Tula' ],
        birthday: new Date(1969, 6, 16),
        isMerriedFirstTime: true,
        phones: [ '+122233344553', '+122233344554' ] }
];

if (JSON.stringify(target) !== JSON.stringify(etalon)) {
    console.error(JSON.stringify(target));
    console.error(JSON.stringify(etalon));
    console.error("Error #3");
    process.exit(1);
}

console.log("ok");
process.exit(0);
