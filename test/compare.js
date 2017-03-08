//var nkit = require('nkit4nodejs');
var nkit = require('../index.js');
var fs = require('fs');
var xml2js = require('xml2js');
var async = require('async');

/*
    For this example you have to create XML file of following structure:

    <?xml version="1.0"?>
    <commerce>
        <offer>
            <id>150016102</id>
            <realty_type>F</realty_type>
            <deal_type>S</deal_type>
            <location type="П">Адмиралтейский</location>
            <address>3 Красноармейская ул., д.13</address>
            <agency_id>2225</agency_id>
            <area total="147.0" />
            <currency>RUR</currency>
            <price>12012000</price>
            <photo>http://img.eip.ru/img/obj/e/08/59/18949192.jpg</photo>
            <note>Любое назначение, 2 отдельных входа с улицы. ПП. Возможна сдача помещения в аренду.</note>
        </offer>
        ...
        <offer>
        ...
        </offer>
    </commerce>

    Don't create huge file: ~20Mb is enough.
    You can use this file: https://www.dropbox.com/s/7417kugfwopf792/big.xml?dl=0
*/

var xmlFile = "/path/to/your/file.xml";

var mapping = ["/offer",
    {
        "/id": "string",
        "/realty_type": "string",
        "/deal_type": "string",
        "/location": "string",
        "/address": "string",
        "/agency_id": "string",
        "/area/@total -> totalArea": "number",
        "/currency": "string",
        "/price": "number",
        "/photo": "string",
        "/note": "string"
    }
];

var mappings = {"main": mapping}

async.series([
    // measuring nkit4nodejs performance
    function(callback){
//        callback(null);
    	console.log("Bench nkit");
        var start = new Date();
        var builder = new nkit.Xml2VarBuilder(mappings);
        var rstream = fs.createReadStream(xmlFile);
        rstream
            .on('data', function (chunk) {
                builder.feed(chunk);
            })
            .on('end', function () {  // done
                var result = builder.end();
                var end = new Date() - start;
                console.log("Offers count: %d", result["main"].length);
                console.info("Execution time: %d ms", end);
                console.info("")
                callback(null);
            });
    },
    // measuring xml2js performance
    function(callback){
//        callback(null);
    	console.log("Bench xml2js");
        var start = new Date();
        var parser = new xml2js.Parser({
            explicitArray: false,
            explicitRoot: false
        });
        fs.readFile(xmlFile, function(err, data) {
            parser.parseString(data, function (err, result) {
                var end = new Date() - start;
                console.log("Offers count: %d", result.offer.length);
                console.info("Execution time: %d ms", end);
                console.info("")
                callback(null);
            });
        });
    },
]);
