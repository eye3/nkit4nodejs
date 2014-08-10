var nkit = require('../index.js');
var fs = require('fs');

var xmlString = fs.readFileSync(__dirname + "/data/sample.xml");

// build list-of-lists-of-strings from xml string
var mapping = ["/person", ["/phone", "string"]];
var gen = new nkit.Xml2VarBuilder(mapping);
gen.feed(xmlString); // can be more than one call to feed(xmlChunk) method
var target = gen.end();
console.log(JSON.stringify(target, null, '  ')); // prints list of lists of string

var etalon = [ [ '+122233344550', '+122233344551' ],
    [ '+122233344553', '+122233344554' ] ];

if (JSON.stringify(target) !== JSON.stringify(etalon)) {
    console.error("Error #1");
    process.exit(1);
}

// build list-of-objects-with-lists from xml string
mapping = ["/person",
    {
        "/birthday": "datetime|1970-01-01|%Y-%m-%d",
        "/phone -> phones": ["/", "string"],
        "/address -> cities": ["/city", "string"], // same as "/address/city -> cities": ["/", "string"]
        "/married/@firstTime -> isMerriedFirstTime": "boolean"
    }
];
gen = new nkit.Xml2VarBuilder(mapping);
gen.feed(xmlString); // can be more than one call to feed(xmlChunk) method
target = gen.end();
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
    console.error("Error #2");
    process.exit(1);
}

// build object from xml string (last 'person' element will be used)
mapping = {
    "/person/name -> lastPersonName": "string",
    "/person/married/@firstTime -> lastPersonIsMarriedFirstTime": "boolean"
};
gen = new nkit.Xml2VarBuilder(mapping);
gen.feed(xmlString); // can be more than one call to feed(xmlChunk) method
target = gen.end();
console.log(JSON.stringify(target, null, '  '));

console.log("ok");
process.exit(0);
