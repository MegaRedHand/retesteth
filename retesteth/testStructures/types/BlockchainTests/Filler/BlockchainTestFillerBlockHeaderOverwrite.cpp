#include "BlockchainTestFillerBlockHeaderOverwrite.h"
#include <testStructures/Common.h>
using namespace dataobject;

namespace test
{
namespace teststruct
{
BlockHeaderOverwrite::BlockHeaderOverwrite(DataObject const& _data)
{
    spDataObject tmpD = convertDecBlockheaderIncompleteToHex(_data);
    if ((*tmpD).getSubObjects().size() > 0)
        m_blockHeaderIncomplete = spBlockHeaderIncomplete(new BlockHeaderIncomplete(tmpD));

    m_relTimeStamp = 0;
    if (_data.count("RelTimestamp"))
    {
        m_hasRelTimeStamp = true;
        m_relTimeStamp = hexOrDecStringToInt(_data.atKey("RelTimestamp").asString());
    }
    if (_data.count("forceNoWithdrawalsRLP"))
        m_forceNoWithdrawalsRLP = true;
}

}  // namespace teststruct
}  // namespace test
