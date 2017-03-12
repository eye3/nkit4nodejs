//var nkit = require('nkit4nodejs');
var nkit = require('../index.js');
var fs = require('fs');
var xml2js = require('xml2js');
var rapidx2j = require('rapidx2j');
var xml2js_expat = require('xml2js-expat');
var async = require('async');

var fileContents = "<?xml version='1.0' encoding='utf-8'?><commerce>";
var element = [
		'<offer>',
		'  <id>150016102</id>',
		'  <boolean>True</boolean>',
		'  <string>String</string>',
		'  <note>qe qwe qwe qwe qw qe qwe wqe qwe qwe qwe qwe qwe qew q</note>',
		'</offer>' ].join('\n');
var footer = "</commerce>";
for (var i = 0; i < 300000; i++)
	fileContents += element
fileContents += footer;
console.info("fileContents length: %d M", fileContents.length
		/ (1024.0 * 1024.0));

async.series([

function(callback) {
	console.log("Bench rapidx2j");
	var start = new Date();
	var result = rapidx2j.parse(fileContents);
	var end = new Date() - start;
	console.info("Execution time: %d ms", end);
	console.info(result["offer"][0])
	console.info("")
	callback(null);
},

function(callback) {
	console.log("Bench nkit with mappings");
	var mapping = [ "/offer", {
		"/id" : "integer",
		"/string" : "string",
		"/boolean" : "boolean",
		"/note" : "string"
	} ];
	var mappings = {
		"main" : mapping
	}
	var builder = new nkit.Xml2VarBuilder(mappings);
	var start = new Date();
	builder.feed(fileContents);
	var result = builder.end()["main"];
	var end = new Date() - start;
	console.info("Execution time: %d ms", end);
	console.info(result[0])
	console.info("")
	callback(null);
},
//*
function(callback) {
	console.log("Bench nkit w/o mappings");
	var options = {
		attrkey : "$",
		textkey : "_",
		trim : true,
		explicit_array : false
	}

	var start = new Date();
	var builder = new nkit.AnyXml2VarBuilder(options);
	builder.feed(fileContents);
	var result = builder.end();
	var end = new Date() - start;
	console.info("Execution time: %d ms", end);
	console.info(result["offer"][0])
	console.info("")
	callback(null);
},

function(callback) {
	console.log("Bench xml2js-expat");
	var parser = new xml2js.Parser(function(result, error) {
		if (!error) {
			console.log(util.inspect(result));
		} else {
			console.error(error);
		}
		console.log('Done.');
	});
	var start = new Date();
	parser.parseString(fileContents);
	var end = new Date() - start;
	console.info("Execution time: %d ms", end);
	console.info("")
	callback(null);
},

function(callback) {
	console.log("Bench xml2js");
	var start = new Date();
	var parser = new xml2js.Parser({
		explicitArray : false,
		explicitRoot : false
	});
	parser.parseString(fileContents, function(err, result) {
		var end = new Date() - start;
		console.info("Execution time: %d ms", end);
		console.info(result["offer"][0])
		console.info("")
		callback(null);
	});
},
//*/
]);
