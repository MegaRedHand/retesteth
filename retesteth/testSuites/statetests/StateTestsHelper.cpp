#include "StateTestsHelper.h"
#include <retesteth/ExitHandler.h>
#include <retesteth/Options.h>
#include <retesteth/TestOutputHelper.h>

using namespace std;
using namespace test::teststruct;
using namespace test::debug;

namespace test::statetests
{
string const c_trHashNotFound = "TR hash not found in mined block! (Check that tr is properly mined and not oog)";
bool OptionsAllowTransaction(TransactionInGeneralSection const& _tr)
{
    Options const& opt = Options::get();
    if ((opt.trData.index == (int)_tr.dataInd() || opt.trData.index == -1) &&
        (opt.trGasIndex == (int)_tr.gasInd() || opt.trGasIndex == -1) &&
        (opt.trValueIndex == (int)_tr.valueInd() || opt.trValueIndex == -1) &&
        (opt.trData.label == _tr.transaction()->dataLabel() || opt.trData.label.empty()))
        return true;
    return false;
}

void checkUnexecutedTransactions(std::vector<TransactionInGeneralSection> const& _txs, Report _report)
{
    bool atLeastOneExecuted = false;
    bool atLeastOneWithoutExpectSection = false;
    for (auto const& tr : _txs)
    {
        if (ExitHandler::receivedExitSignal())
            return;
        if (tr.getExecuted())
            atLeastOneExecuted = true;
        bool transactionExecutedOrSkipped = tr.getExecuted() || tr.getSkipped();
        atLeastOneWithoutExpectSection = !transactionExecutedOrSkipped || atLeastOneWithoutExpectSection;
        if (!transactionExecutedOrSkipped || atLeastOneWithoutExpectSection)
        {
            TestInfo errorInfo("all", (int)tr.dataInd(), (int)tr.gasInd(), (int)tr.valueInd());
            TestOutputHelper::get().setCurrentTestInfo(errorInfo);
            ETH_MARK_ERROR("Test has transaction uncovered with expect section!");
        }
    }
    if (!atLeastOneExecuted)
    {
        Options const& opt = Options::get();
        TestInfo errorInfo(
            opt.singleTestNet.empty() ? "N/A" : opt.singleTestNet.c_str(), opt.trData.index, opt.trGasIndex, opt.trValueIndex);
        TestOutputHelper::get().setCurrentTestInfo(errorInfo);
    }

    if (!atLeastOneExecuted)
    {
        string const errorMessage = "Specified filter did not run a single transaction! " + TestOutputHelper::get().testInfo().errorDebug();
        if (_report == Report::ERROR)
        {
            ETH_ERROR_MESSAGE(errorMessage);
        }
        else
        {
            if (Options::isLegacy())
            {
                ETH_DC_MESSAGEC(DC::LOWLOG, errorMessage, LogColor::YELLOW);
            }
            else
            {
                ETH_WARNING(errorMessage);
            }
        }
    }
}

}  // namespace test::statetests
