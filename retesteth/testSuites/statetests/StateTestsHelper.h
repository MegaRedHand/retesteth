#pragma once
#include <retesteth/session/Session.h>
#include <retesteth/testStructures/structures.h>

namespace test::statetests
{
enum class Report
{
    WARNING,
    ERROR
};
extern std::string const c_trHashNotFound;
bool OptionsAllowTransaction(TransactionInGeneralSection const& _tr);
void checkUnexecutedTransactions(std::vector<TransactionInGeneralSection> const& _txs, Report _report = Report::WARNING);

void RunTest(StateTestInFilled const& _test);
spDataObject FillTest(StateTestInFiller const& _test);
spDataObject FillTestAsBlockchain(StateTestInFiller const& _test);


struct StateTestExecInfo
{
    StateTestExecInfo(test::session::SessionInterface& _session,
        StateTestInFilled const& _test,
        StateTestPostResult const& _res,
        TransactionInGeneralSection& _tr,
        FORK const& _net)
      : session(_session), test(_test), expResult(_res),  tr(_tr), network(_net) {}
    test::session::SessionInterface& session;
    StateTestInFilled const& test;
    StateTestPostResult const& expResult;
    TransactionInGeneralSection& tr;
    FORK network;
};

struct StateTestFillerExecInfo
{
    StateTestFillerExecInfo(test::session::SessionInterface& _session,
        StateTestInFiller const& _test,
        StateTestFillerExpectSection const& _res,
        TransactionInGeneralSection& _tr,
        FORK const& _net)
      : session(_session), test(_test), expResult(_res),  tr(_tr), network(_net) {}
    test::session::SessionInterface& session;
    StateTestInFiller const& test;
    StateTestFillerExpectSection const& expResult;
    TransactionInGeneralSection& tr;
    FORK network;
};

}  // namespace test::statetests
