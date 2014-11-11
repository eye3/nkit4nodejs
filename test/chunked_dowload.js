var nkit = require('../index.js');
var fs = require('fs');

var xmlFile = "../../../data/commerce.xml";

var mapping = ["/offer", {
        "/id": "string",
        "/realty_type": "string",
        "/deal_type": "string",
        "/*": {
            "/city": "string",
            "/street": "string"
        }
    }];

var builder = new nkit.Xml2VarBuilder({"any_name": mapping});
var rstream = fs.createReadStream(xmlFile);
rstream
    .on('data', function (chunk) {
        builder.feed(chunk);
        var list = builder.get("any_name"); // get currently constructed list
        console.log(list.length);
        //list.splice(0); // uncomment this to clear list every time
    })
    .on('end', function () {
        var result = builder.end()["any_name"];
        console.log("Items count: %d", result.length);
    });
