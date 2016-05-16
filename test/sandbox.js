var nkit = require('../index.js');
var deep_equal = require('./deep_equal.js');
var fs = require('fs');
var buffer = require('buffer');

var xmlString = fs.readFileSync(__dirname + "/data/short_sample.xml");

var options = {
    "trim": true,
    "attrkey": "$",
    "textkey": "_"
}

var builder = new nkit.AnyXml2VarBuilder(options);
builder.feed(xmlString);
var result = builder.end();
var root_element_name = builder.root_name()

console.log(root_element_name);

options = {
	    "rootname": "persons",
	    "encoding": "UTF-8",
	    "xmldec": {
	        "version": "1.0",
	        "standalone": true,
	    },
	    "as_buffer": false,
	    "priority": ["name",
	                 "phone"
	    ],
	    "pretty": {
	        "indent": "    ",
	        "newline": "\n",
	    },
	    "attrkey": "$",
	    "textkey": "_",
	}

xmlString = nkit.var2xml(result, options)
console.log(xmlString);

process.exit(0);
