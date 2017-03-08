//var nkit = require('nkit4nodejs');
var nkit = require('../index.js');
var fs = require('fs');

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

var builder = new nkit.Xml2VarBuilder({"main": mapping});
var rstream = fs.createReadStream(xmlFile);
rstream
    .on('data', function (chunk) {
        builder.feed(chunk);
        var list = builder.get("main"); // get currently constructed list
        console.log(list.length);
        //list.splice(0); // uncomment this to clear list every time
    })
    .on('end', function () {
        var result = builder.end()["main"];
        console.log("Items count: %d", result.length);
    });
