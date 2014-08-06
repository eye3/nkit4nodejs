var nkit = require('../index.js');
var fs = require('fs');

console.log();

var fieldsMap = fs.readFileSync(__dirname + "/data/list_of_lists.json");
var xmlString = fs.readFileSync(__dirname + "/data/sample.xml");
var gen = new nkit.Xml2VarBuilder(fieldsMap);
gen.feed(xmlString);
var target = gen.end();
var etalon = [ [ '+122233344550', '+122233344551' ],
    [ '+122233344553', '+122233344554' ] ];

if (JSON.stringify(target) !== JSON.stringify(etalon)) {
    console.error("#1");
    process.exit(1);
}

var fieldsMap = fs.readFileSync(
        __dirname + "/data/list_of_objects_with_list.json");
var gen = new nkit.Xml2VarBuilder(fieldsMap);
gen.feed(xmlString);
var target = gen.end();

etalon = [
    { birthday: new Date(1980, 10, 28),
        isMerriedFirstTime: false,
        phones: [ '+122233344550', '+122233344551' ] },

    { birthday: new Date(1979, 6, 16),
        isMerriedFirstTime: true,
        phones: [ '+122233344553', '+122233344554' ] }
];

if (JSON.stringify(target) !== JSON.stringify(etalon)) {
    console.error("#2");
    process.exit(1);
}

console.log("ok");
process.exit(0);
