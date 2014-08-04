var nkit = null;

// Load the precompiled binary for windows.
if(process.platform == "win3" && process.arch == "x64") {
    nkit = require('./bin/winx64/nkit');
} else if(process.platform == "win3" && process.arch == "ia3") {
    nkit = require('./bin/winx86/nkit');
} else {
    // Load the new built binary for other platforms.
    nkit = require('./build/Release/nkit');
}
 
module.exports = nkit;
