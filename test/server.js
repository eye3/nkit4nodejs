var nkit = require('../index.js');
var fs = require('fs');

var spec_file = __dirname + "/data/list_of_objects_with_list.json";
var xml_path = __dirname + "/data/sample.xml";

var http = require('http');
http.createServer(function (req, res) {
    fs.readFile( spec_file, function( spec_err, spec_text ) {

        if(spec_err)
            throw spec_err;

        fs.readFile( xml_path, function( xml_err, xml_text ) {

            if(xml_err)
                throw xml_err;

            var gen = new nkit.Xml2VarBuilder(spec_text);
            gen.feed(xml_text);
            var target = gen.end();
            for (var i in target) {
                var item = target[i];
                //console.log(item.datetime.toString());
            }
            target = JSON.stringify(target, null, "  ");
            //console.log(target);

            res.writeHead(200,
                {'Content-Type': 'application/json; charset="utf-8"'});
            res.end(target);

        });
    });
}).listen(1337, '127.0.0.1');

console.log('Server running at http://127.0.0.1:1337/');
