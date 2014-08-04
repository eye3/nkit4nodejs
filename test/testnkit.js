var nkit = require('../index.js');
var fs = require('fs');

console.log();

var fieldsMap = fs.readFileSync(__dirname + "/data/list_of_lists.json");
var xmlString = fs.readFileSync(__dirname + "/data/sample.xml");
var gen = new nkit.Xml2VarBuilder(fieldsMap);
gen.feed(xmlString);
var target = gen.end();
console.log(target);

var fieldsMap = fs.readFileSync(
        __dirname + "/data/list_of_objects_with_list.json");
var gen = new nkit.Xml2VarBuilder(fieldsMap);
gen.feed(xmlString);
var target = gen.end();
console.log(target);
