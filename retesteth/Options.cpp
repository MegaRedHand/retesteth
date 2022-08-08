/*
    This file is part of cpp-ethereum.

    cpp-ethereum is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cpp-ethereum is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file
 * Class for handling testeth custom options
 */

#include <iostream>
#include <iomanip>

#include <dataObject/ConvertFile.h>
#include <retesteth/Options.h>
#include <retesteth/TestHelper.h>
#include <testStructures/Common.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <retesteth/dataObject/SPointer.h>

using namespace std;
using namespace test;
namespace fs = boost::filesystem;
Options::DynamicOptions Options::m_dynamicOptions;
void displayTestSuites();

void printVersion()
{
    cout << prepareVersionString() << "\n";
}

void printHelp()
{
    printVersion();
    cout << "Usage: \n";
    cout << std::left;
    cout << "\nSetting test suite\n";
    cout << setw(30) << "-t <TestSuite>" << setw(0) << "Execute test operations\n";
    cout << setw(30) << "-t <TestSuite>/<TestCase>" << setw(0) << "\n";
    cout << "\nAll options below must follow after `--`\n";
    cout << "\nRetesteth options\n";
    cout << setw(40) << "-j <ThreadNumber>" << setw(0) << "Run test execution using threads\n";
    cout << setw(40) << "--clients `client1, client2`" << setw(0)
         << "Use following configurations from datadir path (default: ~/.retesteth)\n";
    cout << setw(40) << "--datadir" << setw(0) << "Path to configs (default: ~/.retesteth)\n";
    cout << setw(40) << "--nodes" << setw(0) << "List of client tcp ports (\"addr:ip, addr:ip\")\n";
    cout << setw(42) << " " << setw(0) << "Overrides the config file \"socketAddress\" section \n";
    cout << setw(40) << "--help -h" << setw(25) << "Display list of command arguments\n";
    cout << setw(40) << "--version -v" << setw(25) << "Display build information\n";
    cout << setw(40) << "--list" << setw(25) << "Display available test suites\n";

    cout << "\nSetting test suite and test\n";
    cout << setw(40) << "--testpath <PathToTheTestRepo>" << setw(25) << "Set path to the test repo\n";
    cout << setw(40) << "--testfile <TestFile>" << setw(0) << "Run tests from a file. Requires -t <TestSuite>\n";
    cout << setw(40) << "--testfolder <SubFolder>" << setw(0) << "Run tests from a custom test folder located in a given suite. Requires -t <TestSuite>\n";
    cout << setw(40) << "--outfile <TestFile>" << setw(0) << "When using `--testfile` with `--filltests` output to this file\n";
    cout << setw(40) << "--singletest <TestName>" << setw(0)
         << "Run on a single test. `Testname` is filename without Filler.json\n";
    cout << setw(40) << "--singletest <TestName>/<Subtest>" << setw(0) << "`Subtest` is a test name inside the file\n";
    cout << setw(40) << "--singlenet <ForkName>" << setw(0) << "Run only specific fork configuration\n";

    cout << "\nDebugging\n";
    cout << setw(30) << "-d <index>" << setw(25) << "Set the transaction data array index when running GeneralStateTests\n";
    cout << setw(30) << "-d <label>" << setw(25) << "Set the transaction data array label (string) when running GeneralStateTests\n";
    cout << setw(30) << "-g <index>" << setw(25) << "Set the transaction gas array index when running GeneralStateTests\n";
    cout << setw(30) << "-v <index>" << setw(25) << "Set the transaction value array index when running GeneralStateTests\n";
    cout << setw(30) << "--vmtrace" << setw(25) << "Trace transaction execution\n";
    cout << setw(30) << "--vmtraceraw" << setw(25) << "Trace transaction execution raw format\n";
    cout << setw(30) << "--vmtraceraw <file>" << setw(25) << "Trace transaction execution raw format to a given file\n";
    cout << setw(30) << "--vmtrace.nomemory" << setw(25) << "Disable memory in vmtrace/vmtraceraw\n";
    cout << setw(30) << "--vmtrace.nostack" << setw(25) << "Disable stack in vmtrace/vmtraceraw\n";
    cout << setw(30) << "--vmtrace.noreturndata" << setw(25) << "Disable returndata in vmtrace/vmtraceraw\n";
    cout << setw(30) << "--limitblocks" << setw(25) << "Limit the block exectuion in blockchain tests for debug\n";
    cout << setw(30) << "--limitrpc" << setw(25) << "Limit the rpc exectuion in tests for debug\n";
    cout << setw(30) << "--verbosity <level>" << setw(25) << "Set logs verbosity. 0 - silent, 1 - only errors, 2 - informative, >2 - detailed\n";
    cout << setw(30) << "--nologcolor" << setw(25) << "Disable color codes in log output\n";
    cout << setw(30) << "--exectimelog" << setw(25) << "Output execution time for each test suite\n";
//    cout << setw(30) << "--statediff" << setw(25) << "Trace state difference for state tests\n";
    cout << setw(30) << "--stderr" << setw(25) << "Redirect ipc client stderr to stdout\n";
    cout << setw(30) << "--travisout" << setw(25) << "Output `.` to stdout\n";

    cout << "\nAdditional Tests\n";
    cout << setw(30) << "--all" << setw(0) << "Enable all tests\n";
    cout << setw(30) << "--lowcpu" << setw(25) << "Disable cpu intense tests\n";

    cout << "\nTest Generation\n";
    cout << setw(30) << "--filltests" << setw(0) << "Run test fillers\n";
    cout << setw(30) << "--fillchain" << setw(25) << "When filling the state tests, fill tests as blockchain instead\n";
    cout << setw(30) << "--showhash" << setw(25) << "Show filler hash debug information\n";
    cout << setw(30) << "--checkhash" << setw(25) << "Check that tests are updated from fillers\n";
    cout << setw(30) << "--poststate" << setw(25) << "Debug show test postState hash or fullstate\n";
    cout << setw(30) << "--fullstate" << setw(25) << "Do not compress large states to hash\n";
    cout << setw(30) << "--forceupdate" << setw(25) << "Update generated test (_info) even if there are no changes\n";

    //	cout << setw(30) << "--randomcode <MaxOpcodeNum>" << setw(25) << "Generate smart random EVM
    //code\n"; 	cout << setw(30) << "--createRandomTest" << setw(25) << "Create random test and
    //output it to the console\n"; 	cout << setw(30) << "--createRandomTest <PathToOptions.json>" <<
    //setw(25) << "Use following options file for random code generation\n"; 	cout << setw(30) <<
    //"--seed <uint>" << setw(25) << "Define a seed for random test\n"; 	cout << setw(30) <<
    //"--options <PathTo.json>" << setw(25) << "Use following options file for random code
    //generation\n";  cout << setw(30) << "--fulloutput" << setw(25) << "Disable address compression
    // in the output field\n";
}

Options::Options(int argc, const char** argv)
{
    trDataIndex = -1;
    trGasIndex = -1;
    trValueIndex = -1;
    bool seenSeparator = false;  // true if "--" has been seen.
    for (auto i = 0; i < argc; ++i)
    {
        auto arg = std::string{argv[i]};
        auto throwIfNoArgumentFollows = [&i, &argc, &arg]() {
            if (i + 1 >= argc)
                BOOST_THROW_EXCEPTION(InvalidOption(arg + " option is missing an argument."));
        };
        auto throwIfAfterSeparator = [&seenSeparator, &arg]() {
            if (seenSeparator)
                BOOST_THROW_EXCEPTION(
                    InvalidOption(arg + " option appears after the separator `--`."));
        };
        auto throwIfBeforeSeparator = [&seenSeparator, &arg]() {
            if (!seenSeparator)
                BOOST_THROW_EXCEPTION(
                    InvalidOption(arg + " option appears before the separator `--`"));
        };
        auto getOptionalArg = [&i, &argc, &argv]() {
            if (i + 1 < argc)
            {
                string nextArg = argv[++i];
                if (nextArg.substr(0, 1) != "-")
                    return nextArg;
            }
            return string();
        };

        if (arg == "--")
        {
            if (seenSeparator)
                BOOST_THROW_EXCEPTION(
                    InvalidOption("The separator `--` appears more than once in the command line."));
            seenSeparator = true;
            continue;
        }
        else if (arg == "-t")
        {
            throwIfAfterSeparator();
            throwIfNoArgumentFollows();
            rCurrentTestSuite = std::string{argv[++i]};
            continue;
        }
        else if (i == 0)
        {
            // Skip './retesteth'
            continue;
        }

        if (arg == "--help" || arg == "-h")
        {
            printHelp();
            exit(0);
        }
        else if (arg == "--version" || (arg == "-v" && !seenSeparator))
        {
            printVersion();
            exit(0);
        }

        // Options below are not allowed before -- separator
        throwIfBeforeSeparator();
        if (arg.substr(0, 2) == "-j")
        {
            if (arg.length() != 2)
            {
                string threadDigits = arg.substr(2, arg.length());
                threadCount = max(1, atoi(threadDigits.c_str()));
            }
            else
            {
                throwIfNoArgumentFollows();
                string nextArg = argv[++i];
                if (nextArg.substr(0, 1) != "-")
                    threadCount = max(1, atoi(nextArg.c_str()));
            }
        }
        else if (arg == "--stderr")
        {
            enableClientsOutput = true;
        }
        else if (arg == "--travisout")
        {
            travisOutThread = true;
        }
        else if (arg == "--vm" || arg == "--evmc")
        {
            // Skip VM options because they are handled by vmProgramOptions().
            throwIfNoArgumentFollows();
            ++i;
        }
        else if (arg == "--vmtrace")
        {
            vmtrace = true;
        }
        else if (arg == "--vmtraceraw")
        {
            vmtrace = true;
            vmtraceraw = true;
            vmtracerawfile = getOptionalArg();
        }
        else if (arg == "--vmtrace.nomemory")
        {
            vmtrace_nomemory = true;
        }
        else if (arg == "--vmtrace.nostack")
        {
            vmtrace_nostack = true;
        }
        else if (arg == "--vmtrace.noreturndata")
        {
            vmtrace_noreturndata = true;
        }
        else if (arg == "--jsontrace")
        {
            throwIfNoArgumentFollows();
            jsontrace = true;
            auto arg = std::string{argv[++i]};
            // Json::Value value;
            // Json::Reader().parse(arg, value);
            // jsontraceOptions = debugOptions(value);
        }
        else if (arg == "--filltests")
            filltests = true;
        else if (arg == "--forceupdate")
            forceupdate = true;
        else if (arg == "--limitblocks")
        {
            throwIfNoArgumentFollows();
            blockLimit = atoi(argv[++i]);
        }
        else if (arg == "--limitrpc")
        {
            throwIfNoArgumentFollows();
            rpcLimit = atoi(argv[++i]);
        }
        else if (arg == "--fillchain")
        {
            fillchain = true;

            bool noFilltests = !filltests;
            if (noFilltests)
            {
                // Look ahead if this option ever provided
                for (auto i = 0; i < argc; ++i)
                {
                    auto arg = std::string{argv[i]};
                    if (arg == "--filltests")
                    {
                        noFilltests = false;
                        break;
                    }
                }
            }

            if (noFilltests)
                ETH_STDOUT_MESSAGEC("WARNING: `--fillchain` option provided without `--filltests`, activating `--filltests` (did you mean `--filltests`?)", cYellow);
            filltests = true;
        }
        else if (arg == "--showhash")
            showhash = true;
        else if (arg == "--checkhash")
            checkhash = true;
        else if (arg == "--stats")
        {
            throwIfNoArgumentFollows();
            stats = true;
            statsOutFile = argv[++i];
        }
        else if (arg == "--exectimelog")
            exectimelog = true;
        else if (arg == "--all")
            all = true;
        else if (arg == "--lowcpu")
            lowcpu = true;
        else if (arg == "--singletest")
        {
            throwIfNoArgumentFollows();
            singleTest = true;
            singleTestName = std::string{argv[++i]};

            size_t pos = singleTestName.find("Filler");
            if (pos != string::npos)
            {
                singleTestName = singleTestName.substr(0, pos);
                ETH_STDOUT_MESSAGEC("WARNING: Correcting filter to: `" + singleTestName + "`", cYellow);
            }

            pos = singleTestName.find_last_of('/');
            if (pos != string::npos)
            {
                singleSubTestName = singleTestName.substr(pos + 1);
                singleTestName = singleTestName.substr(0, pos);
            }
        }
        else if (arg == "--testfile")
        {
            throwIfNoArgumentFollows();
            if (customTestFolder.is_initialized())
            {
                ETH_STDERROR_MESSAGE("--testfolder initialized together with --testfile");
                exit(1);
            }
            singleTestFile = std::string{argv[++i]};
            if (!boost::filesystem::exists(singleTestFile.get()))
            {
                ETH_STDERROR_MESSAGE(
                    "Could not locate custom test file: '" + singleTestFile.get() + "'");
                exit(1);
            }
        }
        else if (arg == "--testfolder")
        {
            throwIfNoArgumentFollows();
            if (singleTestFile.is_initialized())
            {
                ETH_STDERROR_MESSAGE("--testfolder initialized together with --testfile");
                exit(1);
            }
            customTestFolder = std::string{argv[++i]};
        }
        else if (arg == "--outfile")
        {
            throwIfNoArgumentFollows();
            singleTestOutFile = std::string{argv[++i]};
        }
        else if (arg == "--singlenet")
        {
            throwIfNoArgumentFollows();
            singleTestNet = std::string{argv[++i]};
        }
        else if (arg == "--fullstate")
            fullstate = true;
        else if (arg == "--poststate")
        {
            poststate = true;
            fullstate = true;
        }
        else if (arg == "--verbosity")
        {
            throwIfNoArgumentFollows();
            static std::ostringstream strCout;  // static string to redirect logs to
            logVerbosity = atoi(argv[++i]);
            if (logVerbosity == 0)
            {
                // disable all output
                std::cout.rdbuf(strCout.rdbuf());
                std::cerr.rdbuf(strCout.rdbuf());
                break;
            }
        }
        else if (arg == "--nologcolor")
        {
            nologcolor = true;
        }
        else if (arg == "--datadir")
        {
            throwIfNoArgumentFollows();
            datadir = fs::path(std::string{argv[++i]});
        }
        else if (arg == "--nodes")
        {
            throwIfNoArgumentFollows();
            for (auto const& el : explode(std::string{argv[++i]}, ','))
                nodesoverride.push_back(IPADDRESS(el));
        }
        else if (arg == "--options")
        {
            throwIfNoArgumentFollows();
            boost::filesystem::path file(std::string{argv[++i]});
            if (boost::filesystem::exists(file))
                randomCodeOptionsPath = file;
            else
            {
                ETH_STDERROR_MESSAGE(
                    "Options file not found! Default options at: "
                    "tests/src/randomCodeOptions.json\n");
                exit(0);
            }
        }
        else if (arg == "--nonetwork")
            nonetwork = true;
        else if (arg == "-d")
        {
            throwIfNoArgumentFollows();
            string const& argValue = argv[++i];
            DigitsType type = stringIntegerType(argValue);
            switch (type)
            {
            case DigitsType::Decimal:
                trDataIndex = atoi(argValue.c_str());
                break;
            case DigitsType::String:
                if (argValue.find(":label") == string::npos)
                    trDataLabel += ":label " + argValue;
                else
                    trDataLabel = argValue;
                break;
            default:
            {
                ETH_STDERROR_MESSAGE("Wrong argument format: " + argValue);
                exit(0);
            }
            }
        }
        else if (arg == "-g")
        {
            throwIfNoArgumentFollows();
            trGasIndex = atoi(argv[++i]);
        }
        else if (arg == "-v")
        {
            throwIfNoArgumentFollows();
            trValueIndex = atoi(argv[++i]);
        }
        else if (arg == "--testpath")
        {
            throwIfNoArgumentFollows();
            ETH_FAIL_REQUIRE_MESSAGE(testpath.empty(),
                "testpath is already set! Make sure that testpath is provided as a first option.");
            testpath = std::string{argv[++i]};
        }
        else if (arg == "--statediff")
            statediff = true;
        else if (arg == "--randomcode")
        {
            throwIfNoArgumentFollows();
            int maxCodes = atoi(argv[++i]);
            if (maxCodes > 1000 || maxCodes <= 0)
            {
                cerr << "Argument for the option is invalid! (use range: 1...1000)\n";
                exit(1);
            }
            // test::RandomCodeOptions options;
            // cout << test::RandomCode::get().generate(maxCodes, options) << "\n";
            exit(0);
        }
        else if (arg == "--createRandomTest")
        {
            createRandomTest = true;
            if (i + 1 < argc)  // two params
            {
                auto options = std::string{argv[++i]};
                if (options[0] == '-')  // not param, another option
                    i--;
                else
                {
                    boost::filesystem::path file(options);
                    if (boost::filesystem::exists(file))
                        randomCodeOptionsPath = file;
                    else
                        BOOST_THROW_EXCEPTION(
                            InvalidOption("Options file not found! Default options at: "
                                          "tests/src/randomCodeOptions.json\n"));
                }
            }
        }
        else if (arg == "--seed")
        {
            throwIfNoArgumentFollows();
            /*u256 input = toInt(argv[++i]);
                    if (input > std::numeric_limits<uint64_t>::max())
                        BOOST_WARN("Seed is > u64. Using u64_max instead.");
                    randomTestSeed =
               static_cast<uint64_t>(min<u256>(std::numeric_limits<uint64_t>::max(), input));*/
        }
        else if (arg == "--clients")
        {
            throwIfNoArgumentFollows();
            vector<string> clientNames;
            string nnn = std::string{argv[++i]};
            boost::split(clientNames, nnn, boost::is_any_of(", "));
            for (auto& it : clientNames)
            {
                boost::algorithm::trim(it);
                if (!it.empty())
                    clients.push_back(it);
            }
        }
        else if (arg == "--list")
        {
            displayTestSuites();
            exit(0);
        }
        else if (seenSeparator)
        {
            cerr << "Unknown option: " + arg << "\n";
            exit(1);
        }
    }

    // check restrickted options
    if (createRandomTest)
    {
        if (trValueIndex >= 0 || trGasIndex >= 0 || trDataIndex >= 0 || nonetwork || singleTest ||
            all || stats || filltests || fillchain)
        {
            cerr << "--createRandomTest cannot be used with any of the options: "
                 << "trValueIndex, trGasIndex, trDataIndex, nonetwork, singleTest, all, "
                 << "stats, filltests, fillchain \n";
            exit(1);
        }
    }
    else
    {
        if (randomTestSeed.is_initialized())
            BOOST_THROW_EXCEPTION(
                InvalidOption("--seed <uint> could be used only with --createRandomTest \n"));
    }

    if (threadCount == 1)
        dataobject::GCP_SPointer<int>::DISABLETHREADSAFE();
}

Options const& Options::get(int argc, const char** argv)
{
    static Options instance(argc, argv);
    return instance;
}

void displayTestSuites()
{
    cout << "List of available test suites: \n";
    cout << std::left;
    cout << setw(40) << "-t GeneralStateTests" << setw(0) << "Basic state transition tests\n";
    cout << setw(40) << "-t BCGeneralStateTests" << setw(0) << "Basic state transition tests in blockchain form\n";
    cout << setw(40) << "-t BlockchainTests" << setw(0) << "All Blockchain tests\n";
    cout << setw(40) << "-t BlockchainTests/ValidBlocks" << setw(0) << "Subset of correct blocks\n";
    cout << setw(40) << "-t BlockchainTests/InvalidBlocks" << setw(0) << "Subset of malicious blocks\n";
    cout << setw(40) << "-t BlockchainTests/TransitionTests" << setw(0) << "Subset of fork transition tests\n";
    cout << setw(40) << "-t BlockchainTests/ValidBlocks/VMTests" << setw(0)
         << "VMTests converted\n";
    cout << "(Use --filltests to generate the tests, --fillchain to generate BCGeneralStateTests)\n";

    cout << "\nLegacy test suites (Frontier .. ConstantinopleFix):\n";
    cout << setw(55) << "-t LegacyTests" << setw(0) << "All Legacy tests\n";
    cout << setw(55) << "-t LegacyTests/Constantinople" << setw(0) << "Subset of Frontier .. Constantinople tests\n";
    cout << setw(55) << "-t LegacyTests/Constantinople/GeneralStateTests" << setw(0) << "Old state tests\n";
    cout << setw(55) << "-t LegacyTests/Constantinople/BCGeneralStateTests" << setw(0) << "Old state tests in blockchain form\n";
    cout << setw(55) << "-t LegacyTests/Constantinople/BlockchainTests" << setw(0) << "Old blockchain tests\n";

    cout << "\nRetesteth unit tests:\n";
    cout << setw(30) << "-t DataObjectTestSuite" << setw(0) << "Unit tests for json parsing\n";
    cout << setw(30) << "-t EthObjectsSuite" << setw(0) << "Unit tests for test data objects\n";
    cout << setw(30) << "-t LLLCSuite" << setw(0) << "Unit tests for external lllc compiler\n";
    cout << setw(30) << "-t SOLCSuite" << setw(0) << "Unit tests for solidity support\n";
    cout << setw(30) << "-t OptionsSuite" << setw(0) << "Unit tests for this cmd menu\n";
    cout << setw(30) << "-t TestHelperSuite" << setw(0) << "Unit tests for retesteth logic\n";
    cout << "\n";
}

string Options::getGStateTransactionFilter() const
{
    string filter;
    filter += trDataIndex == -1 ? string() : " dInd: " + to_string(trDataIndex);
    filter += trDataLabel.empty() ? string() : " dLbl: " + trDataLabel;
    filter += trGasIndex == -1 ? string() : " gInd: " + to_string(trGasIndex);
    filter += trValueIndex == -1 ? string() : " vInd: " + to_string(trValueIndex);
    return filter;
}

bool Options::isLegacy()
{
    static bool isLegacy = (boost::unit_test::framework::current_test_case().full_name().find("LegacyTests") != string::npos);

    // Current test case is dynamic if we run all tests. need to see if we hit LegacyTests
    if (Options::get().rCurrentTestSuite.empty())
        isLegacy = (boost::unit_test::framework::current_test_case().full_name().find("LegacyTests") != string::npos);

    return isLegacy;
}
