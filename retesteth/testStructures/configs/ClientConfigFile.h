#pragma once
#include "../basetypes/IPADDRESS.h"
#include "FORK.h"
#include <retesteth/dataObject/DataObject.h>
#include <retesteth/dataObject/SPointer.h>
#include <boost/filesystem.hpp>
#include <map>
using namespace dataobject;
namespace fs = boost::filesystem;

namespace test
{
namespace teststruct
{
enum class ClientConfgSocketType
{
    TCP,
    IPC,
    IPCDebug,
    TransitionTool
};

struct ClientConfigFile : GCP_SPointerBase
{
    ClientConfigFile(DataObject const& _data);
    ClientConfigFile(fs::path const& _clientConfigPath);

    string const& name() const { return m_name; }
    ClientConfgSocketType socketType() const { return m_socketType; }
    std::vector<IPADDRESS> const& socketAdresses() const;
    std::map<string, fs::path> const& customCompilers() const { return m_customCompilers; }
    size_t initializeTime() const { return  m_initializeTime; }
    std::vector<FORK> const& forks() const { return m_forks; }
    std::vector<FORK> const& additionalForks() const { return m_additionalForks; }
    std::set<FORK> allowedForks() const;
    std::set<FORK> forkProgressionAsSet() const;
    bool checkLogsHash() const { return m_checkLogsHash; }
    int chainID() const { return m_chanID; }

    std::map<string, string> const& exceptions() const { return m_exceptions; }
    std::map<string, string> const& fieldreplace() const { return m_fieldRaplce; }
    fs::path const& path() const { return m_configFilePath; }
    fs::path const& shell() const { return m_pathToExecFile; }


private:
    ClientConfigFile() {}
    void initWithData(DataObject const&);
    void parseSocketType(DataObject const& _data, string const& _sErrorPath);

private:
    // Inside the file
    string m_name;                           ///< Client name
    ClientConfgSocketType m_socketType;      ///< Connection type
    std::vector<IPADDRESS> m_socketAddress;  ///< List of IP to connect to (IP::PORT)

    std::map<string, fs::path> m_customCompilers;
    bool m_checkLogsHash;                    ///< Enable logsHash verification
    int m_chanID;                            ///< Use custom chainID

    size_t m_initializeTime;                 ///< Time to start the instance
    std::vector<FORK> m_forks;               ///< Allowed forks as network name
    std::vector<FORK> m_additionalForks;     ///< Allowed forks as network name
    std::map<string, string> m_exceptions;   ///< Exception Translation
    std::map<string, string> m_fieldRaplce;  ///< Replace field names in requests map

    // Additional values
    fs::path m_configFilePath;  ///< Path to the config file
    fs::path m_pathToExecFile;  ///< Path to cmd that runs the client instance (for t8ntool)
};


}  // namespace teststruct
}  // namespace test
