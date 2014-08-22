var nkit = require('../index.js');
var fs = require('fs');

var mapping = '["/person", \
{ \
    "/birthday": "datetime|1970-01-01|%Y-%m-%d",          \
    "/phone -> phones": ["/", "string"],                  \
    "/address -> cities": ["/city", "string"],            \
    "/married/@firstTime -> isMerriedFirstTime": "boolean"\
} \
]';

var xml_path = __dirname + "/data/sample.xml";

var http = require('http');
http.createServer(function (req, res) {
    fs.readFile( xml_path, function( xml_err, xml_text ) {

        if(xml_err)
            throw xml_err;

        var gen = new nkit.Xml2VarBuilder(mapping);
        gen.feed(xml_text);
        var target = gen.end();
        for (var i in target)
            var item = target[i];

        target = JSON.stringify(target, null, "  ");

        res.writeHead(200,
            {'Content-Type': 'application/json; charset="utf-8"'});
        res.end(target);

    });
}).listen(1337, '127.0.0.1');

console.log('Server running at http://127.0.0.1:1337/');
