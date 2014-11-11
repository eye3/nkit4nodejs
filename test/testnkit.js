var nkit = require('../index.js');
var deep_equal = require('./deep_equal.js');
var fs = require('fs');
var buffer = require('buffer');

var xmlString = fs.readFileSync(__dirname + "/data/sample.xml");

var etalons = {};
var mappings = {};


function add_test(name, mapping, etalon) {
    mappings[name] = mapping;
    etalons[name] = etalon;
}

//------------------------------------------------------------------------------
// build list-of-strings from xml string

// Here mapping is list, described by /path/to/element and list item
// description. List item here is a 'string' scalar.
// Scalar definition contains type name and optional default value.
add_test("list_of_strings",
    ["/person/phone", "string"],
    [
        '+122233344550',
        '+122233344551',
        '+122233344553',
        '+122233344554']);

// -----------------------------------------------------------------------------
// Building a list-of-lists-of-strings from xml string
//
// Here mapping is list, described by /path/to/element and list item
// description. List item is described as 'list' sub-mapping, described
// by sub-path and'string' scalar definition
add_test("list_of_list_of_strings",
    ["/person", ["/phone", "string"]],
    [
        ['+122233344550', '+122233344551'],
        ['+122233344553', '+122233344554']
    ]);

// -----------------------------------------------------------------------------
// Building a simple object from xml string (last 'person' element will be used)
//
// Here mapping is object, described by set of mappings, each containing
// key definition and scalar definition.˛
// Keys are described by "/sub/path -> optionalKeyName".
// If optionalKeyName doesn't provided, then last element name in /sub/path
// will be used for key name.
// Scalar definition may have optional "...|defaultValue"

add_test("single_object",
    {
        "/person/name -> lastPersonName": "string|Captain Nemo",
        "/person/married/@firstTime -> lastPersonIsMarriedFirstTime": "boolean|true",
        "/person/age": "integer"
    }, {
        "age": 34,
        "lastPersonName": "Boris",
        "lastPersonIsMarriedFirstTime": true
    });

// -----------------------------------------------------------------------------
// Building list-of-objects-with-lists from xml string
//
// Here mapping is list, described by /path/to/element and list item description.
//  List item is described as 'object' sub-mapping.
//  This 'object' sub-mapping described by set of mappings, each containing
//  key definition and sub-mapping or scalar.
//  Keys are described by "/sub/path -> optionalKeyName".
//  If optionalKeyName doesn't provided, then last element name in "/sub/path"
//  will be used for key name
//  Scalar definition may have optional "...|defaultValue"
//  'datetime' scalar definition MUST contain default value and formatting string

add_test("list_of_objects_with_lists", [
    "/person",
    {
        "/birthday": "datetime|Fri, 22 Aug 2014 13:59:06 +0000|%a, %d %b %Y %H:%M:%S %z",
        "/phone -> phones": ["/", "string"],
        "/address -> cities": ["/city", "string"],
            // same as "/address/city -> cities": ["/", "string"]
        "/photos": ["/*", "string"],
        "/married/@firstTime -> isMerriedFirstTime": "boolean"
    }],
    [
        {
            "isMerriedFirstTime": false,
            "phones": ['+122233344550', '+122233344551'],
            "photos": ["img1", "img2", "img3"],
            "birthday": new Date(1979, 2, 28, 12, 13, 14),
            "cities": ['New York', 'Boston']
        },
        {
            "isMerriedFirstTime": true,
            "phones": ['+122233344553', '+122233344554'],
            "photos": ["img3", "img4"],
            "birthday": new Date(1970, 7, 31, 2, 3, 4),
            "cities": ['Moscow', 'Tula']
        }
    ]);

// -----------------------------------------------------------------------------
// Testing default values

add_test("testing_default_values",
    ["/person", {
        "/key_for_default_value": "string|default_value",
        "/non_existing_key": "string"
    }],
    [
        {
            "key_for_default_value": "default_value"
        },
        {
            "key_for_default_value": "default_value"
        }
    ]);

// -----------------------------------------------------------------------------
// Testing empty values

add_test("testing_empty_values",
    ["/person", {
        "/empty": "string"
    }],
    [
        {
            "empty": ""
        },
        {
            "empty": ""
        }
    ]);

// -----------------------------------------------------------------------------
// Testing new object creation

add_test("testing_new_object_creation",
    ["/person", {
        "/empty": {
            "/ -> k1": "string",
            "/ -> k2": "string"
        }
    }],
    [{
        "empty": {
            "k2": "",
            "k1": ""
        }
    },
    {
        "empty": {
            "k2": "",
            "k1": ""
        }
    }]);

// -----------------------------------------------------------------------------
var builder = new nkit.Xml2VarBuilder(mappings);
builder.feed(xmlString);
var res = builder.end();

counter = 0;
for (name in etalons) {
    counter += 1;
    if (!deep_equal.deepEquals(res[name], etalons[name])) {
        console.error("ETALON of " + name + ":");
        console.error(JSON.stringify(etalons[name], null, 2));
        console.error("RESULT of " + name + ":");
        console.error(JSON.stringify(res[name], null, 2));
        console.error("Error #1." + str(counter));
        process.exit(1);
    }
}

// -----------------------------------------------------------------------------
// mappings as string
mappings = '{"map_name": ["/person/phone", "string"]}';

builder = new nkit.Xml2VarBuilder(mappings);
builder.feed(xmlString);
res = builder.end()["map_name"];

etalon = ['+122233344550',
          '+122233344551',
          '+122233344553',
          '+122233344554'];

if (!deep_equal.deepEquals(res, etalon)) {
    console.error(JSON.stringify(res, null, 2));
    console.error(JSON.stringify(etalon, null, 2));
    console.error("Error #1");
    process.exit(1);
}

// mappings buffer
mappings = buffer.Buffer('{"map_name": ["/person/phone", "string"]}');

builder = new nkit.Xml2VarBuilder(mappings);
builder.feed(xmlString);
res = builder.end()["map_name"];


if (!deep_equal.deepEquals(res, etalon)) {
    console.error(JSON.stringify(res, null, 2));
    console.error(JSON.stringify(etalon, null, 2));
    console.error("Error #1.1");
    process.exit(1);
}

if (!deep_equal.deepEquals(res, builder.get("map_name"))) {
    console.error(JSON.stringify(res, null, 2));
    console.error(JSON.stringify(builder.get("map_name"), null, 2));
    console.error("Error #1.2");
    process.exit(1);
}

// -----------------------------------------------------------------------------
data = [{
    "$": {"p1": "v1 < > & \" '", "p2": "v2"},
    "_": "Hello world",
    "int": 1,
    "float": 1.123456789,
    "cdata": "text < > & \" '",
    "list": [[1, 2.333], 2, 3],
    "datetime": new Date(1979, 1, 28, 12, 13, 14),
    "dict": {
        "$": {"a1": "V1", "a2": "V2"},
        "sub_int": 1,
        "sub_float": 1.11234567891234,
        "sub_string": "text < > & \" '",
        "sub_list": [[1], 2, 3]
    }
},{
    "$": {"p1": "v1 < > & \" '", "p2": "v2"},
    "_": "Hello world",
    "int": 1,
    "float": 1.123456789,
    "cdata": "text < > & \" '",
    "list": [[1, 2.333], 2, 3],
    "datetime": new Date(1979, 1, 28, 12, 13, 14),
    "dict": {
        "$": {"a1": "V1", "a2": "V2"},
        "sub_int": 1,
        "sub_float": 1.11234567891234,
        "sub_string": "text < > & \" '",
        "sub_list": [[1], 2, 3]
    }
}];

options = {
//    "rootname": "ROOT",
    "itemname": "item",
    "xmldec": {
        "version": "1.0",
        "encoding": "UTF-8",
        "standalone": true
    },
    "pretty": {
        "indent": "  ",
        "newline": "\n"
    },
    "attrkey": "$",
    "textkey": "_",
    "cdata": ["cdata", "float"],
    "float_precision": 10,
    "date_time_format": "%Y-%m-%d %H:%M:%S %z"
};

console.log(nkit.var2xml(data, options).toString());

console.log("ok");
process.exit(0);

// -----------------------------------------------------------------------------
// Testing paths with '*'
mapping = ["/person",
    {
        "/*": "string"
    }
];

mapping_name = "testing_paths_with_star";

var options = {"trim": true};

builder = new nkit.Xml2VarBuilder(options, {mapping_name: mapping});
builder.feed(xmlString);
res = builder.end()[mapping_name];

etalon = [
    {
        "name": "Jack",
        "photos": "",
        "age": "33",
        "married": "Yes",
        "phone": "+122233344551",
        "birthday": "Wed, 28 Mar 1979 12:13:14 +0300",
        "address": "",
        "empty": ""
    },
    {
        "name": "Boris",
        "photos": "",
        "age": "34",
        "married": "Yes",
        "phone": "+122233344554",
        "birthday": "Mon, 31 Aug 1970 02:03:04 +0300",
        "address": "",
        "empty": ""
    }
];

if (!deep_equal.deepEquals(res, etalon)) {
    console.error(JSON.stringify(res, null, 2));
    console.error(JSON.stringify(etalon, null, 2));
    console.error("Error #4.1");
    process.exit(1);
}

console.log("ok");
process.exit(0);
