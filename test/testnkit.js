var nkit = require('../index.js');
var fs = require('fs');

console.log(new Date("1970-11-27T00:00:00").toString());

var xmlString = fs.readFileSync(__dirname + "/data/sample.xml");

var gen = new nkit.Xml2VarBuilder(["/person", ["/phone", "string"]]);
gen.feed(xmlString);
var target = gen.end();

console.log(target);

var etalon = [ [ '+122233344550', '+122233344551' ],
    [ '+122233344553', '+122233344554' ] ];

if (JSON.stringify(target) !== JSON.stringify(etalon)) {
    console.error("Error #1");
    process.exit(1);
}

var gen = new nkit.Xml2VarBuilder(["/person",
    {
        "/birthday": "datetime|1970-01-01|%Y-%m-%d",
        "/phone -> phones": ["/", "string"],
        "/married/@firstTime -> isMerriedFirstTime": "boolean"
    }
]);

gen.feed(xmlString);
var target = gen.end();

console.log(target);

etalon = [
    {
        birthday: new Date(1970, 10, 28),
        isMerriedFirstTime: false,
        phones: [ '+122233344550', '+122233344551' ] },

    {
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

console.log("ok");
process.exit(0);
