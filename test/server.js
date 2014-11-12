var cluster = require('cluster');

if (cluster.isMaster) {
    // Keep cluster from automatically grabbing stdin/out/err
    cluster.setupMaster({ silent: false });

    // Fork workers.
    var workers = {};
    for (var i = 0; i < 3; i++) {
        var worker = cluster.fork();
        workers[worker.process.pid] = worker;
    }
} else {
    var nkit = require('../index.js');
    var fs = require('fs');
    var url = require('url');
    var http = require('http');

    var mapping = ["/person",
        {
            "/birthday": "datetime|1970-01-01|%Y-%m-%d",
            "/phone -> phones": ["/", "string"],
            "/address -> cities": ["/city", "string"],
            "/married/@firstTime -> isMerriedFirstTime": "boolean"
        }];

    var xml_path = __dirname + "/data/sample.xml";

    DATA = {
        "$": {"p1": "в1&v2\"'", "p2": "v2"},
        "_": "Hello(Привет) world(мир)",
        "int_число": 1,
        "true": true,
        "false": false,
        "float": 1.123456789,
        "cdata1": "text < > & \" '",
        "cdata2": "%^&*()-=+ < > & \" '",
        "list": [[1, 2], 2, 3],
        "datetime": new Date(1979, 2, 28, 12, 13, 14),
        "dict": {
            "$": {"a1": "V1", "a2": "V2"},
            "int": 1,
            "float": 1.11234567891234,
            "sub_string": "text < > & \" '",
            "list": [[1], 2, 3]
        }
    };

    ENC = "UTF-8";
    //ENC = "cp1251";

    OPTIONS = {
        "rootname": "ROOT",
        "itemname": "item",
        "xmldec": {
            "version": "1.0",
            "encoding": ENC,
            "standalone": true
        },
        "pretty": {
            "indent": "  ",
            "newline": "\n"
        },
        "attrkey": "$",
        "textkey": "_",
        "cdata": ["cdata1", "cdata2"],
        "float_precision": 10,
        "date_time_format": "%Y-%m-%d %H:%M:%S",
        "bool_true": "Yes",
        "bool_false": "No"
    };

    http.createServer(function (req, res) {
        var parsed_url = url.parse(req.url, true);
        var pathname = parsed_url.pathname;
        if (pathname.indexOf("/xml2var") > -1) {
            fs.readFile( xml_path, function( xml_err, xml_text ) {

                if(xml_err)
                    throw xml_err;

                var gen = new nkit.Xml2VarBuilder({"": mapping});
                gen.feed(xml_text);
                var target = gen.end()[""];
                for (var i in target)
                    var item = target[i];

                target = JSON.stringify(target, null, "  ");

                res.writeHead(200,
                    {'Content-Type': 'application/json; charset="utf-8"'});
                res.end(target);

            });
        } else if (pathname.indexOf("/var2xml") > -1) {
            res.writeHead(200,
                {'Content-Type': 'text/xml; charset="' + ENC + '"'});
            res.end(nkit.var2xml(DATA, OPTIONS));
        }
    }).listen(1337, '127.0.0.1');

    console.log('Server running at http://127.0.0.1:1337/');
}
