var nkit = null;

// Load the precompiled binary for windows.
if(process.platform == "win32" && process.arch == "x64") {
    nkit = require(__dirname + '/bin/winx64/nkit4nodejs.node');
} else if(process.platform == "win32" && process.arch == "ia32") {
    nkit = require(__dirname + '/bin/winx86/nkit4nodejs.node');
} else {
    // Load the new built binary for other platforms.
    console.log(__dirname + '/build/Release/nkit4nodejs.node');
    nkit = require(__dirname + '/build/Release/nkit4nodejs.node');
}

module.exports = nkit;
