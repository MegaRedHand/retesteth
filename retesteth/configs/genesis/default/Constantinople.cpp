#include <retesteth/configs/Options.h>
using namespace std;
using namespace dataobject;

namespace retesteth::options
{
const string t8ntool_Constantinople_config = R"({
    "params" : {
        "fork" : "Constantinople",
        "constantinopleForkBlock" : "0x00",
        "byzantiumForkBlock" : "0x00",
        "homesteadForkBlock" : "0x00",
        "chainID" : "0x01"
    },
    "accounts" : {
    }
})";

genConstantinopleCfg::genConstantinopleCfg()
{
    spDataObject obj2;
    (*obj2)["path"] = "default/genesis/Constantinople.json";
    (*obj2)["content"] = t8ntool_Constantinople_config;
    map_configs.addArrayObject(obj2);
}
}  // namespace retesteth::options
