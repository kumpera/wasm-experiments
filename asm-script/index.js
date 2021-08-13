const fs = require("fs");
const loader = require("@assemblyscript/loader");

example_features = new Map()

function read_feature(ns, n) {
    var k = wasmModule.exports.__getString(ns) + ":" + wasmModule.exports.__getString(n);
    console.log("read_feature " + k);
    var val = example_features.get(k)
    return val === undefined ? 0 : val
}

function write_feature(ns, n, v)
{
    var k = wasmModule.exports.__getString(ns) + ":" + wasmModule.exports.__getString(n);
    console.log("write_feature " + k + " = " + v)
    example_features[k] = v
}

const imports = { 
    "index": {
        "vw.read_feature": read_feature,
        "vw.write_feature": write_feature
    }
};

const wasmModule = loader.instantiateSync(fs.readFileSync(__dirname + "/build/optimized.wasm"), imports);
module.exports = wasmModule.exports;

try {
    // console.log(module.exports);
    module.exports.process_example()
} catch(e) {
    console.log(e)
}