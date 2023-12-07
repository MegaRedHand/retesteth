﻿#include "helpers/TestHelper.h"
#include "StateTestsHelper.h"
#include "StateTestFillerRunner.h"
#include <retesteth/helpers/TestOutputHelper.h>
#include "Options.h"
#include "session/Session.h"
#include "testSuites/Common.h"
#include "testStructures/PrepareChainParams.h"
#include <retesteth/testSuiteRunner/TestSuite.h>
#include <testStructures/types/Ethereum/Transactions/TransactionLegacy.h>

using namespace test;
using namespace std;
using namespace test::debug;
using namespace test::session;
using namespace test::statetests;

namespace
{

string const testInclude = R"(
import pytest

from ethereum_test_forks import Fork, Frontier, Homestead, Berlin, London, Shanghai, Merge, Cancun
from ethereum_test_tools import Account, Code, Environment
from ethereum_test_tools import StateTestFiller, TestAddress, Transaction

)";


string const expectInclude = R"(
from collections import namedtuple
from typing import Any, Mapping, Optional

import pytest

from ethereum_test_forks import Fork, Frontier, Homestead, Berlin, London, Shanghai, Merge, Cancun
from ethereum_test_tools import Account, Code, Environment
from ethereum_test_tools import StateTestFiller, TestAddress, Transaction


_BaseExpectSectionIndex = namedtuple("ExpectSectionIndex", ["d", "g", "v", "f", "l"])

class ExpectSectionIndex(_BaseExpectSectionIndex):
    __slots__ = ()  # Optional, to save memory

    def __new__(cls, d, g, v, f, l=None):
        return super(ExpectSectionIndex, cls).__new__(cls, d, g, v, f, l)


class ExpectSection:
    """Manage expected post states for state tests transactions"""

    def __init__(self):
        self.sections: list[tuple[ExpectSectionIndex, dict[str, Account]]] = []

    def add_expect(self, ind: ExpectSectionIndex, expect: dict[str, Account]) -> None:
        """Adds a section with a given indexes and expect dictionary."""
        self.sections.append((ind, expect))

    def get_expect(self, tx_ind: ExpectSectionIndex) -> Optional[dict[str, Account]]:
        """Returns the element associated with the given id, if it exists."""
        for section_ind, section in self.sections:
            if (
                ((tx_ind.l == "" and tx_ind.d in section_ind.d)
                     or (tx_ind.l != "" and tx_ind.l in section_ind.d)
                     or -1 in section_ind.d)
                and (tx_ind.g in section_ind.g or -1 in section_ind.g)
                and (tx_ind.v in section_ind.v or -1 in section_ind.v)
                and (tx_ind.f in section_ind.f)
            ):
                return section
        return None
)";

}

namespace test::statetests
{

std::string InsertTab(size_t _number, std::string _str, bool _exeptFirstLast = false)
{
    std::string tabs(_number * 4, ' ');

    if (!_exeptFirstLast)
        _str.insert(0, tabs);
    for (size_t i = 0; i < _str.size() - 1; ++i)
    {
        if (_str[i] == '\n' && _str[i + 1] != '\n')
        {
            if (_exeptFirstLast && _str.find('\n', i + 1) == string::npos)
                break;
            _str.insert(i + 1, tabs);
            i += _number;
        }
    }
    return _str;
}

string convertComment(StateTestInFiller const& _test, TestSuite::TestSuiteOptions& _opt)
{
    if (_test.hasInfo())
    {
        string res = "\"\"\"\n";
        res += "Converted test: " + _opt.pathToFiller.string() + "\n";
        res += "Retesteth version: " + prepareVersionString() + "\n";
        res +=  _test.Info().rawData()->atKey("comment").asString() + "\n";
        res += "\"\"\"\n" ;
        return res;
    }
    return string();
}

string defineDataParameter(StateTestInFiller const& _test)
{
    string dataParameters;
    string datas;
    auto const& dataVector = _test.GeneralTr().databoxVector();
    for (size_t i = 0; i < dataVector.size(); i++)
    {
        datas += "    (" + to_string(i) + ", "
                 + "\"" + dataVector.at(i).m_data.asString() + "\", "
                 + "\"" + dataVector.at(i).m_dataLabel + "\""
                 + "),\n";

    }
    datas.erase(datas.rfind(",\n"));
    dataParameters = "@pytest.mark.parametrize(\"tr_data\", [\n" + datas + "\n])\n";
    return dataParameters;
}

string defineGasParameter(StateTestInFiller const& _test)
{
    string gasParameters;
    string gasLimits;
    auto const& gasVector = _test.GeneralTr().gasLimitVector();
    for (size_t i = 0; i < gasVector.size(); i++)
        gasLimits += "    (" + to_string(i) + ", " + gasVector.at(i).asDecString() + "),\n";
    gasLimits.erase(gasLimits.rfind(",\n"));
    gasParameters = "@pytest.mark.parametrize(\"tr_gasLimit\", [\n" + gasLimits + "\n])\n";
    return gasParameters;
}

string defineValueParameter(StateTestInFiller const& _test)
{
    string valueParameters;
    string values;
    auto const& valueVector = _test.GeneralTr().valueVector();
    for (size_t i = 0; i < valueVector.size(); i++)
        values += "    (" + to_string(i) + ", " + valueVector.at(i).asDecString() + "),\n";
    values.erase(values.rfind(",\n"));
    valueParameters = "@pytest.mark.parametrize(\"tr_value\", [\n" + values + "\n])\n";
    return valueParameters;
}

string defineForksParameter(StateTestInFiller const& _test)
{
    /*string forks;
    for (auto const& fork : _test.getAllForksFromExpectSections())
        forks += "\"" + fork.asString() + "\", ";
    forks.erase(forks.rfind(','));

    string fork = "@pytest.mark.parametrize(\"fork\", [" + forks + "])\n";*/

    auto allForksInTest = _test.getAllForksFromExpectSections();
    auto const& allforks = Options::getCurrentConfig().cfgFile().forks();
    FORK lowestFork("notdefined");
    for (auto fork : allforks)
    {
        // assume the test is chronological fork order
        if (allForksInTest.count(fork))
        {
            lowestFork = fork;
            break;;
        }
    }
    string forkstr = "@pytest.mark.valid_from(\"" + lowestFork.asString() + "\")\n";
    return forkstr;
}

string defineTestName(StateTestInFiller const& _test)
{
    string res;
    res += "def test_" + _test.testName() + "_py(\n";
    res += "    env: Environment,\n";
    res += "    pre: dict,\n";
    if (_test.Expects().size() > 1)
        res += "    expect: ExpectSection,\n";
    else
        res += "    post: dict,\n";
    res += "    fork: Fork,\n";
    res += "    tr_data,\n";
    res += "    tr_gasLimit,\n";
    res += "    tr_value,\n";
    res += "    state_test: StateTestFiller):\n";
    return res;
}

string defineCommentAfterTestName(StateTestInFiller const& _test)
{
    string res = "\"\"\"\n";
    if (_test.hasInfo())
        res += _test.Info().rawData()->atKey("comment").asString() + "\n";
    else
        res += "Converted into python state test" + _test.testName() + "\n";
    res += "\"\"\"\n\n";
    return res;
}

string defineEnvironment(StateTestInFiller const& _test)
{
    auto const& env = _test.Env();
    string res = R"(
@pytest.fixture
def env():  # noqa: D103
    return Environment(
        coinbase=")" + env.currentCoinbase().asString() + R"(",
        difficulty=)" + env.currentDifficulty().asString() + R"(,
        gas_limit=)" + env.currentGasLimit().asDecString() + R"(,
        number=)" + env.currentNumber().asDecString() + R"(,
        timestamp=)" + env.firstBlockTimestamp().asDecString() + R"(,
    ))";
    res += "\n\n";
    return res;
}

void defineAccount(string& _res, spAccountBase _acc, size_t _additionalTabs = 0)
{
    auto const& addr = _acc->address();
    if (_acc->shouldNotExist())
    {
        _res.insert(_res.end(), 4 * _additionalTabs, ' ');
        _res += "    \"" + addr.asString() + "\": Account.NONEXISTENT,\n";
        return;
    }

    _res.insert(_res.end(), 4 * _additionalTabs, ' ');
    _res += "    \"" + addr.asString() + "\": Account(\n";
    if (_acc->hasBalance())
    {
        _res.insert(_res.end(), 4 * _additionalTabs, ' ');
        _res += "        balance=" + _acc->balance().asDecString() + ",\n";
    }
    if (_acc->hasNonce())
    {
        _res.insert(_res.end(), 4 * _additionalTabs, ' ');
        _res += "        nonce=" + _acc->nonce().asDecString() + ",\n";
    }
    if (_acc->hasCode())
    {
        _res.insert(_res.end(), 4 * _additionalTabs, ' ');
        _res += "        code=\"" + _acc->code().asString() + "\",\n";
    }
    if (_acc->hasStorage())
    {
        _res.insert(_res.end(), 4 * _additionalTabs, ' ');
        _res += "        storage="
                + InsertTab(2 + _additionalTabs, _acc->storage().asDataObject()->asJsonNoFirstKey(), true)
                + ",\n";
        _res.insert(_res.end() - 3, 8 + 4 * _additionalTabs, ' ');
    }
    _res.insert(_res.end(), 4 * _additionalTabs, ' ');
    _res += "    ),\n";
}

string definePre(StateTestInFiller const& _test)
{
    string res = "@pytest.fixture\n";
    res += "def pre():  # noqa: D103\n";
    res += "    return {\n";
    for (auto const& [addr, acc] : _test.Pre().accounts())
        defineAccount(res, acc, 1);
    res += "    }\n\n";
    return res;
}

string defineTransactionStructure(spTransaction _tr)
{
    string res;
    if (_tr->type() == TransactionType::LEGACY)
    {
        auto const& legacyTr = static_cast<TransactionLegacy const&>(_tr.getCContent());
        res = R"(tx = Transaction(
        ty=0x0,
        chain_id=0x0,
        nonce=)" + legacyTr.nonce().asDecString() + R"(,
        to=")" + legacyTr.to().asString() + R"(",
        gas_price=)" + legacyTr.gasPrice().asDecString() + R"(,
        protected=False,
))";
        res += "\n\n";
    }
    return res;
}

string defineTransaction(StateTestInFiller const& _test)
{
    string res;
    auto txs = _test.GeneralTr().buildTransactions();
    res += defineTransactionStructure(txs.at(0).transaction());
    res += "dataInd, dataValue, dataLabel = tr_data\n";
    res += "gasInd, gasValue = tr_gasLimit\n";
    res += "valueInd, valueValue = tr_value\n\n";

    res += "tx_index = ExpectSectionIndex(d=dataInd, l=dataLabel, g=gasInd, v=valueInd, f=fork)\n";
    res += "post=expect.get_expect(tx_index)\n\n";

    res += "tx.data=dataValue\n";
    res += "tx.gas_limit=gasValue\n";
    res += "tx.value=valueValue\n";
    res += "state_test(env=env, pre=pre, post=post, txs=[tx])\n\n";
    return res;
}

string defineSinglePost(StateTestInFiller const& _test)
{
    string res;
    res = "post = {\n";
    for (auto const& [addr, acc] : _test.Expects().at(0).result().accounts())
        defineAccount(res, acc);
    res += "}\n\n";
    return res;
}

string defineExpectPosts(StateTestInFiller const& _test)
{
    (void)_test;
    string res;
    res = "@pytest.fixture\n";
    res += "def expect():  # noqa: D103\n";
    res += "    expect_section = ExpectSection()\n";
    for (auto const& expect : _test.Expects())
    {


        string dataLabelStr;
        string dataIndStr;
        auto const& databox = _test.GeneralTr().databoxVector();
        for (auto const& dataInd : expect.getDataInd())
        {
            dataIndStr += to_string(dataInd) + ", ";
            auto const& label = databox.at(dataInd).m_dataLabel;
            if (!label.empty())
            {
                if (dataLabelStr.find(label) == string::npos)
                    dataLabelStr += "\"" + label + "\", ";
            }
        }
        string gasIndStr;
        for (auto const& gasInd : expect.getGasInd())
            gasIndStr += to_string(gasInd) + ", ";
        string valueIndStr;
        for (auto const& valueInd : expect.getValInd())
            valueIndStr += to_string(valueInd) + ", ";
        string forkStr;
        for (auto const& fork : expect.forks())
            forkStr += fork.asString() + ", ";

        auto lastDataLabelCommaPos = dataLabelStr.rfind(", ");
        if (lastDataLabelCommaPos != string::npos)
            dataLabelStr.erase(lastDataLabelCommaPos, 2);

        dataIndStr.erase(dataIndStr.rfind(", "), 2);
        gasIndStr.erase(gasIndStr.rfind(", "), 2);
        valueIndStr.erase(valueIndStr.rfind(", "), 2);
        forkStr.erase(forkStr.rfind(", "), 2);

        res += "    expect_section.add_expect(\n";
        res += "        ExpectSectionIndex(\n";
        if (dataLabelStr.empty())
            res += "            d=[" + dataIndStr + "],\n";
        else
            res += "            d=[" + dataLabelStr + "],\n";
        res += "            g=[" + gasIndStr + "],\n";
        res += "            v=[" + valueIndStr + "],\n";
        res += "            f=[" + forkStr + "]\n";
        res += "        ),\n";

        res += "        {\n";
        for (auto const& [addr, acc] : expect.result().accounts())
            defineAccount(res, acc, 2);
        res += "        }\n";

        res += "    )\n";
    }
    res += "    return expect_section\n\n";
    return res;
}

spDataObject ConvertpyTest(StateTestInFiller const& _test, TestSuite::TestSuiteOptions& _opt)
{
    spDataObject res = sDataObject(DataType::String);
    auto& s = (*res).asStringUnsafe();
    s += convertComment(_test, _opt);

    if (_test.Expects().size() == 1)
        s += testInclude + "\n";
    else
        s += expectInclude + "\n";

    s += defineEnvironment(_test);
    s += definePre(_test);
    if (_test.Expects().size() == 1)
        s += defineSinglePost(_test);
    else
        s += defineExpectPosts(_test);


    s += defineForksParameter(_test);
    s += defineDataParameter(_test);
    s += defineGasParameter(_test);
    s += defineValueParameter(_test);
    s += defineTestName(_test);

    s += InsertTab(1, defineCommentAfterTestName(_test));
    s += InsertTab(1, defineTransaction(_test));

    (void)_test;
    return res;
}
}
