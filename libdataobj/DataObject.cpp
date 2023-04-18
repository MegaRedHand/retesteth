#include "DataObject.h"
#include <math.h>
#include <algorithm>
#include <iostream>
#include <sstream>
using namespace dataobject;
using namespace std;

/// Default dataobject is null
DataObject::DataObject() { m_type = DataType::NotInitialized; }

/// Define dataobject of _type, pass the value later (will check the value and _type)
DataObject::DataObject(DataType _type) { m_type = _type; }

/// Define dataobject of string
DataObject::DataObject(std::string&& _str) : m_value(std::move(_str)) { m_type = DataType::String; }
DataObject::DataObject(std::string const& _str) : m_value(_str) { m_type = DataType::String; }

/// Define dataobject[_key] = string
DataObject::DataObject(std::string&& _key, std::string&& _str)
  : m_strKey(std::move(_key)), m_value(std::move(_str))
{
    m_type = DataType::String;
}
DataObject::DataObject(std::string const& _key, std::string const& _str)
  : m_strKey(_key), m_value(_str)
{
    m_type = DataType::String;
}

DataObject::DataObject(std::string&& _key, int _val)
  : m_strKey(std::move(_key)), m_value(_val)
{
    m_type = DataType::Integer;
}

/// Define dataobject of int
DataObject::DataObject(int _int) : m_value(_int) { m_type = DataType::Integer; }

/// Define dataobject of bool
DataObject::DataObject(DataType type, bool _bool) : m_value(_bool) { m_type = type; }

/// Get dataobject type
DataType DataObject::type() const { return m_type; }

/// Set key of the dataobject
void DataObject::setKey(std::string&& _key) { m_strKey = std::move(_key); }
void DataObject::setKey(std::string const& _key) { m_strKey = _key; }

/// Get key of the dataobject
std::string const& DataObject::getKey() const { return m_strKey; }
std::string& DataObject::getKeyUnsafe() { return m_strKey; }

/// Get vector of subobjects
std::vector<spDataObject> const& DataObject::getSubObjects() const
{
    return m_subObjects;
}

/// Get map of keys to subobjects
std::map<string, spDataObject> const& DataObject::getSubObjectKeys() const
{
    return m_subObjectKeys;
}

/// Get ref vector of subobjects
std::vector<spDataObject>& DataObject::getSubObjectsUnsafe()
{
    return m_subObjects;
}

/// Add new subobject
DataObject& DataObject::addSubObject(spDataObject const& _obj)
{
    return _addSubObject(_obj);
}

/// Add new subobject and set it's key
DataObject& DataObject::addSubObject(std::string&& _key, spDataObject const& _obj)
{
    return _addSubObject(_obj, std::forward<string&&>(_key));
}
DataObject& DataObject::addSubObject(std::string const& _key, spDataObject const& _obj)
{
    return _addSubObject(_obj, string(_key));
}

/// Set key for subobject _index
void DataObject::setSubObjectKey(size_t _index, std::string&& _key)
{
    _assert(_index < m_subObjects.size(), "_index < m_subObjects.size() (DataObject::setSubObjectKey)");
    if (m_subObjects.size() > _index)
        m_subObjects.at(_index).getContent().setKey(std::forward<string&&>(_key));
}


/// look if there is a subobject with _key
bool DataObject::count(std::string const& _key) const
{
    return m_subObjectKeys.count(_key);
}

/// Get string value
std::string const& DataObject::asString() const
{
    _assert(m_type == DataType::String, "m_type == DataType::String (DataObject::asString())");
    return std::get<std::string>(m_value);
}
std::string& DataObject::asStringUnsafe()
{
    _assert(m_type == DataType::String, "m_type == DataType::String (DataObject::asStringUnsafe())");
    return std::get<std::string>(m_value);
}

std::string const DataObject::asStringAnyway() const
{
    switch(m_type)
    {
    case DataType::String: return asString();
    case DataType::Integer: return to_string(asInt());
    case DataType::Bool: return asBool() ? "true" : "false";
    case DataType::Array: return "Array";
    case DataType::Object: return "Object";
    case DataType::Null: return "Null";
    case DataType::NotInitialized: return "Not initialized";
    default: return "N/A";
    }
}

/// Get int value
int DataObject::asInt() const
{
    _assert(m_type == DataType::Integer, "m_type == DataType::Integer (DataObject::asInt())");
    return std::get<int>(m_value);
}

/// Get bool value
bool DataObject::asBool() const
{
    _assert(m_type == DataType::Bool, "m_type == DataType::Bool (DataObject::asBool())");
    return std::get<bool>(m_value);
}

/// Set position in vector of the subobject with _key
void DataObject::setKeyPos(std::string const& _key, size_t _pos)
{
    //_assert(_pos < m_subObjects.size(), "DataObject::setKeyPos(`" + _key + "`, `" + to_string(_pos) + "`) _pos <
    //m_subObjects.size() ");
    _assert(count(_key), "count(_key) _key = " + _key + " (DataObject::setKeyPos)");
    _assert(!_key.empty(), "!_key.empty() (DataObject::setKeyPos)");

    size_t elementPos = 0;
    for (size_t i = 0; i < m_subObjects.size(); i++)
        if (m_subObjects.at(i)->getKey() == _key)
        {
            if (i == _pos)
                return;  // item already at _pos;
            else
            {
                elementPos = i;
                break;
            }
        }

    setOverwrite(true);
    spDataObject data = m_subObjects.at(elementPos);
    m_subObjects.erase(m_subObjects.begin() + elementPos);
    if (_pos >= m_subObjects.size())
        m_subObjects.push_back(data);
    else
        m_subObjects.insert(m_subObjects.begin() + _pos, 1, data);
    setOverwrite(false);
}


/// replace this object with _value
void DataObject::replace(DataObject const& _value)
{
    m_strKey = _value.getKey();
    switch (_value.type())
    {
    case DataType::String:
        m_value = _value.asString();
        break;
    case DataType::Integer:
        m_value = _value.asInt();
        break;
    case DataType::Bool:
        m_value = _value.asBool();
        break;
    default:
        break;
    }

    m_type = _value.type();
    m_subObjects.clear();
    m_subObjects = _value.getSubObjects();
    m_subObjectKeys = _value.getSubObjectKeys();
    m_allowOverwrite = _value.isOverwritable();
    setAutosort(_value.isAutosort());
}

DataObjectK& DataObjectK::operator=(spDataObject const& _value)
{
    if (m_data.count(m_key))
    {
        m_data.removeKey(m_key);
        if (_value->getKey().empty())
            throw DataObjectException("DataObjectK::operator=(spDataObject const& _value)  _value without key, but key required!");
        m_data.addSubObject(string(_value->getKey()), _value);
    }
    else
        m_data.addSubObject(string(m_key), _value);
    return *this;
}

spDataObject& DataObject::atKeyPointerUnsafe(std::string const& _key)
{
    if (m_subObjectKeys.count(_key))
        return m_subObjectKeys.at(_key);
    _assert(false, "count(_key) _key=" + _key + " (DataObject::atKeyPointerUnsafe)");
    return m_subObjectKeys.at(0);
}

DataObjectK DataObject::atKeyPointer(std::string const& _key)
{
    return DataObjectK(_key, *this);
}

DataObject const& DataObject::atKey(std::string const& _key) const
{
    if (m_subObjectKeys.count(_key))
        return m_subObjectKeys.at(_key).getCContent();

    _assert(false, "count(_key) _key=" + _key + " (DataObject::atKey)");
    return m_subObjects.at(0).getCContent();
}

DataObject& DataObject::atKeyUnsafe(std::string const& _key)
{
    if (m_subObjectKeys.count(_key))
        return m_subObjectKeys.at(_key).getContent();
    _assert(false, "count(_key) _key=" + _key + " (DataObject::atKeyUnsafe)");
    return m_subObjects.at(0).getContent();
}

DataObject const& DataObject::at(size_t _pos) const
{
    _assert((size_t)_pos < m_subObjects.size(), "DataObject::at(int) out of range!");
    return m_subObjects[_pos];
}

DataObject& DataObject::atUnsafe(size_t _pos)
{
    _assert((size_t)_pos < m_subObjects.size(), "DataObject::atUnsafe(int) out of range!");
    return m_subObjects[_pos].getContent();
}

DataObject const& DataObject::atLastElement() const
{
    _assert(m_subObjects.size() > 0, "atLastElement()");
    return m_subObjects.at(m_subObjects.size() - 1);
}

DataObject& DataObject::atLastElementUnsafe()
{
    _assert(m_subObjects.size() > 0, "atLastElementUnsafe()");
    return m_subObjects.at(m_subObjects.size() - 1).getContent();
}

void DataObject::addArrayObject(spDataObject const& _obj)
{
    _assert(m_type == DataType::NotInitialized || m_type == DataType::Array,
        "m_type == DataType::NotInitialized || m_type == DataType::Array (DataObject::addArrayObject)");
    m_type = DataType::Array;
    m_subObjects.push_back(_obj);
    m_subObjects.at(m_subObjects.size() - 1).getContent().setAutosort(m_autosort);
}

void DataObject::renameKey(std::string const& _currentKey, std::string&& _newKey)
{
    if (m_strKey == _currentKey)
        m_strKey = _newKey;

    if (m_subObjectKeys.count(_currentKey))
    {
        spDataObject data = m_subObjectKeys.at(_currentKey);
        m_subObjectKeys.erase(_currentKey);
        data.getContent().setKey(_newKey);
        m_subObjectKeys.emplace(_newKey, data);
    }

}

/// vector<element> erase method with `replace()` function
void DataObject::removeKey(std::string const& _key)
{
    _assert(type() == DataType::Object, "type() == DataType::Object");
    for (std::vector<spDataObject>::const_iterator it = m_subObjects.begin();
         it != m_subObjects.end(); it++)
    {
        if ((*it)->getKey() == _key)
        {
            setOverwrite(true);
            m_subObjects.erase(it);
            m_subObjectKeys.erase(_key);
            setOverwrite(false);
            break;
        }
    }

    /*
    bool startReplace = false;
    for (std::vector<DataObject>::iterator it = m_subObjects.begin(); it != m_subObjects.end();
         it++)
    {
        if ((*it).getKey() == _key)
            startReplace = true;
        std::vector<DataObject>::iterator next = it + 1;
        if (startReplace)
        {
            if (next != m_subObjects.end())
                (*it).replace(*next);
            else
            {
                m_subObjects.erase(it);
                break;
            }
        }
    }*/
}

void DataObject::clear(DataType _type)
{
    m_value = false;
    m_strKey = "";
    m_subObjects.clear();
    m_subObjectKeys.clear();
    m_type = _type;
}

void DataObject::setVerifier(void (*f)(DataObject&))
{
    m_verifier = f;
    m_verifier(*this);
}

void DataObject::performModifier(void (*f)(DataObject&), ModifierOption _opt, std::set<string> const& _exceptionKeys)
{
    if (!_exceptionKeys.count(getKey()))
    {
        f(*this);
        if (_opt == ModifierOption::RECURSIVE)
        {
            for (auto& el : m_subObjects)
                el.getContent().performModifier(f, _opt, _exceptionKeys);
        }
    }
}

bool DataObject::performSearch(bool (*f)(DataObject const&)) const
{
    bool res = true;
    for (auto const& el : m_subObjects)
    {
        res = res && !el->performSearch(f);
        if (false)
            break;
    }
    res = res && !f(*this);
    return !res;
}

std::string DataObject::asJsonNoFirstKey() const
{
    return asJson(0, true, true);
}

std::string DataObject::asJson(int level, bool pretty, bool nokey) const
{
    std::ostringstream out;
    auto printLevel = [level, pretty, &out]() -> void {
        if (pretty)
            for (int i = 0; i < level * 4; i++)
                out << " ";
    };

    auto printElements = [this, &out, level, pretty]() -> void {
        for (std::vector<spDataObject>::const_iterator it = this->m_subObjects.begin();
             it < this->m_subObjects.end(); it++)
        {
            if ((*it).isEmpty())
                out << "NaN";
            else
                out << (*it)->asJson(level + 1, pretty);
            if (it + 1 != this->m_subObjects.end())
                out << ",";
            if (pretty)
                out << std::endl;
        }
    };

    string buffer;
    switch (m_type)
    {
    case DataType::NotInitialized:
        printLevel();
        if (!m_strKey.empty() && !nokey)
        {
            if (pretty)
                out << "\"" << m_strKey << "\" : ";
            else
                out << "\"" << m_strKey << "\":";
        }
        out << "notinit";
        break;
    case DataType::Null:
        printLevel();
        if (!m_strKey.empty() && !nokey)
        {
            if (pretty)
                out << "\"" << m_strKey << "\" : ";
            else
                out << "\"" << m_strKey << "\":";
        }
        out << "null";
        //out << "{}";  // why???
        break;
    case DataType::Object:
        if (!m_strKey.empty() && !nokey)
        {
            printLevel();
            if (pretty)
                out << "\"" << m_strKey << "\" : {" << std::endl;
            else
                out << "\"" << m_strKey << "\":{";
        }
        else
        {
            printLevel();
            out << "{";
            if (pretty)
                out << std::endl;
        }
        printElements();
        printLevel();
        out << "}";
        break;
    case DataType::Array:
        if (!m_strKey.empty() && !nokey)
        {
            printLevel();
            if (pretty)
                out << "\"" << m_strKey << "\" : [" << std::endl;
            else
                out << "\"" << m_strKey << "\":[";
        }
        else
        {
            printLevel();
            out << "[";
            if (pretty)
                out << std::endl;
        }
        printElements();
        printLevel();
        out << "]";
        break;
    case DataType::String:
        printLevel();
        if (pretty)
        {
            if (!m_strKey.empty() && !nokey)
                out << "\"" << m_strKey << "\" : ";
        }
        else
        {
            if (!m_strKey.empty() && !nokey)
                out << "\"" << m_strKey << "\":";
        }

        //  threat special chars
        for (auto const& ch: asString())
        {
            if (ch == 10)
                buffer += "\\n";
            else if (ch == 9)
                buffer += "\\t";
            else
                buffer += ch;
        }
        out << "\"" << buffer << "\"";
        break;
    case DataType::Integer:
        printLevel();
        if (!m_strKey.empty() && !nokey)
        {
            if (pretty)
                out << "\"" << m_strKey << "\" : ";
            else
                out << "\"" << m_strKey << "\":";
        }
        out << std::get<int>(m_value);;
        break;
    case DataType::Bool:
        printLevel();
        if (!m_strKey.empty() && !nokey)
        {
            if (pretty)
                out << "\"" << m_strKey << "\" : ";
            else
                out << "\"" << m_strKey << "\":";
        }
        if (std::get<bool>(m_value))
            out << "true";
        else
            out << "false";
        break;
    default:
        out << "unknown " << dataTypeAsString(m_type) << std::endl;
        break;
    }
    return out.str();
}

std::string DataObject::dataTypeAsString(DataType _type)
{
    switch (_type)
    {
    case String:
        return "string";
    case Integer:
        return "int";
    case Array:
        return "array";
    case Bool:
        return "bool";
    case Object:
        return "object";
    case NotInitialized:
        return "notinit";
    case Null:
        return "null";
    default:
        break;
    }
    return "";
}

size_t dataobject::findOrderedKeyPosition(string const& _key, vector<spDataObject> const& _objects)
{
    if (_objects.size() == 0)
        return 0;

    size_t m = 0;
    int L = 0;
    int R = _objects.size() - 1;
    while (L <= R)
    {
        m = floor((L + R) / 2);
        if (_objects.at(m)->getKey() < _key)
            L = m + 1;
        else if (_objects.at(m)->getKey() > _key)
            R = m - 1;
        else
            return m;
    }

    if (_objects.at(m)->getKey() > _key)
        return m;
    else
        return m + 1;

    /*size_t step =  floor((double)_objects.size() / 2);
    size_t guess = step;
    while (step > 0 && guess > 0 && guess < _objects.size())
    {
        if (_objects.at(guess)->getKey() > _key)
        {
            step = floor((double)(guess) / 2);
            guess -= std::max(step, (size_t)1);
        }
        else
        {
            step = floor((double)(_objects.size() - guess) / 2);
            guess += std::max(step, (size_t)1);
        }
    }

    if (guess == _objects.size())
    {
        std::cerr << _key << " - " << guess << std::endl;
        return guess;
    }

    if (_objects.at(guess)->getKey() <= _key)
        guess++;

   // //guess = max(0, (int)guess - 5);
  //  while (guess < _objects.size() && _objects.at(guess)->getKey() <= _key)
  //      guess++;

    std::cerr << _key << " - " << guess << std::endl;
    return guess;
*/
}

DataObject& DataObject::_addSubObject(spDataObject const& _obj, string&& _keyOverwrite)
{
    if (m_type == DataType::NotInitialized)
        m_type = DataType::Object;

    size_t pos;
    string const key = _keyOverwrite.empty() ? _obj->getKey() : _keyOverwrite;
    if (key.empty() || !m_autosort)
    {
        m_subObjects.push_back(_obj);
        pos = m_subObjects.size() - 1;
        if (!_keyOverwrite.empty())
            setSubObjectKey(pos, std::forward<string&&>(_keyOverwrite));
        else
            setSubObjectKey(pos, string(_obj->getKey()));
        m_subObjects.at(pos).getContent().setOverwrite(m_allowOverwrite);
        m_subObjects.at(pos).getContent().setAutosort(m_autosort);
    }
    else
    {
        // find ordered position to insert key
        // better use it only when export as ordered json !!!
        pos = findOrderedKeyPosition(key, m_subObjects);
        if (pos == m_subObjects.size())
            m_subObjects.push_back(_obj);
        else
        {
            setOverwrite(true);
            m_subObjects.insert(m_subObjects.begin() + pos, 1, _obj);
            setOverwrite(false);
        }

        if (!_keyOverwrite.empty())
            m_subObjects.at(pos).getContent().setKey(std::forward<string&&>(_keyOverwrite));
        else
            m_subObjects.at(pos).getContent().setKey(string(_obj->getKey()));
        m_subObjects.at(pos).getContent().setOverwrite(true);
        m_subObjects.at(pos).getContent().setAutosort(m_autosort);
    }
    if (!key.empty())
        m_subObjectKeys.emplace(key, m_subObjects.at(pos));
    return m_subObjects.at(pos).getContent();
}

void DataObject::_assert(bool _flag, std::string const& _comment) const
{
    if (!_flag)
    {
        std::ostringstream out;
        out << "Error in DataObject: " << std::endl;
        out << " key: '" << m_strKey << "'";
        out << " type: '" << dataTypeAsString(m_type) << "'" << std::endl;
        out << " assert: " << _comment << std::endl;
        out << asJson() << std::endl;
        throw DataObjectException(out.str());
    }
}

void DataObject::setString(string&& _value)
{
    _assert(m_type == DataType::String || m_type == DataType::NotInitialized,
        "In DataObject=(string) DataObject must be string or NotInitialized!");
    m_type = DataType::String;
    m_value = std::move(_value);
}

void DataObject::setInt(int _value)
{
    _assert(m_type == DataType::Integer || m_type == DataType::NotInitialized,
        "In DataObject=(int) DataObject must be int or NotInitialized!");
    m_type = DataType::Integer;
    m_value = _value;
}

void DataObject::setBool(bool _value)
{
    _assert(m_type == DataType::Bool || m_type == DataType::NotInitialized,
        "In DataObject:setBool(bool) DataObject must be bool or NotInitialized!");
    m_type = DataType::Bool;
    m_value = _value;
}

void DataObject::copyFrom(DataObject const& _other)
{
    clear();
    m_type = _other.type();
    if (!_other.getKey().empty())
        m_strKey = _other.getKey();

    switch (m_type)
    {
    case String: m_value = _other.asString(); break;
    case Integer: m_value = _other.asInt(); break;
    case Bool: m_value = _other.asBool(); break;
    case Array:
        for (auto const& el : _other.getSubObjects())
        {
            spDataObject copy = el->copy();
            addArrayObject(copy);
        }
        break;
    case Object:
        for (auto const& el : _other.getSubObjects())
        {
            spDataObject copy = el->copy();
            addSubObject(copy);
        }
        break;
    case Null: break;
    case NotInitialized: break;
    }
}

spDataObject DataObject::copy() const
{
    spDataObject c(new DataObject(m_type));
    if (!m_strKey.empty())
        (*c).setKey(string(m_strKey));
    switch(m_type)
    {
    case String: (*c).setString(string(asString())); break;
    case Integer: (*c).setInt(asInt()); break;
    case Bool: (*c).setBool(asBool()); break;
    case Array:
        for (size_t i = 0; i < m_subObjects.size(); i++)
        {
            spDataObject copy = m_subObjects.at(i)->copy();
            (*c).addArrayObject(copy);
        }
    break;
    case Object:
        for (size_t i = 0; i < m_subObjects.size(); i++)
        {
            spDataObject copy = m_subObjects.at(i)->copy();
            (*c).addSubObject(copy);
        }
    break;
    case Null: break;
    case NotInitialized: break;
    }
    return c;
}

/*DataObject& DataObject::operator=(spDataObject const& _value)
{
    m_myself = _value;

}*/

/*DataObject& DataObject::operator=(DataObject const& _value)
{
    // So not to overwrite the existing data
    // Do not replace the key. Assuming that key is set upon calling DataObject[key] =
    if (!m_allowOverwrite && !m_autosort)
        _assert(m_type == DataType::NotInitialized,
            "m_type == DataType::NotInitialized (DataObject& operator=). Overwriting dataobject that is "
            "not NotInitialized");

    if (m_type != DataType::NotInitialized)
        replace(_value);  // overwrite value and key
    else
    {
        // keep the key "newkey" for object["newkey"] = object2;  declarations when object["newkey"] is NotInitialized;
        string const currentKey = m_strKey;
        replace(_value);
        m_strKey = currentKey;
    }
    return *this;
}*/

bool DataObject::operator==(DataObject const& _value) const
{
    if (type() != _value.type() || getSubObjects().size() != _value.getSubObjects().size())
        return false;
    bool equal = true;
    equal = m_type == _value.type();
    equal = getKey() == _value.getKey();
    switch (m_type)
    {
    case DataType::Bool:
        equal = asBool() == _value.asBool();
        break;
    case DataType::Integer:
        equal = asInt() == _value.asInt();
        break;
    case DataType::String:
        equal = asString() == _value.asString();
        break;
    case DataType::Array:
        equal = getSubObjects().size() == _value.getSubObjects().size();
        for (size_t i = 0; i < getSubObjects().size(); i++)
        {
            equal = getSubObjects().at(i).getCContent() == _value.getSubObjects().at(i).getCContent();
            if (!equal)
                break;
        }
        break;
    case DataType::Object:
        //_assert(getSubObjects().size() == 1,
        //    "in DataObject::==(Object)  LType Object must have only 1 object!");
        //_assert(_value.getSubObjects().size() == 1,
        //    "in DataObject::==(Object)  RType Object must have only 1 object!");
        equal = getSubObjects().size() == _value.getSubObjects().size();
        // equal = getSubObjects().at(0) == _value.getSubObjects().at(0);
        for (size_t i = 0; i < getSubObjects().size(); i++)
        {
            equal = getSubObjects().at(i).getCContent() == _value.getSubObjects().at(i).getCContent();
            if (!equal)
                break;
        }
        break;
    default:
        _assert(false, "in DataObject::== unknown object type!");
        equal = false;
        break;
    }
    return equal;
}

bool DataObject::operator==(bool _value) const
{
    DataObject tmp(DataType::Bool, _value);
    return *this == tmp;
}

DataObject& DataObject::operator=(int _value)
{
    setInt(_value);
    return *this;
}

DataObject& DataObject::operator[](std::string const& _key)
{
    _assert(m_type == DataType::NotInitialized || m_type == DataType::Object,
        "m_type == DataType::NotInitialized || m_type == DataType::Object (DataObject& operator[])");

    if (m_subObjectKeys.count(_key))
        return m_subObjectKeys.at(_key).getContent();

    spDataObject newObj(new DataObject(DataType::NotInitialized));
    newObj.getContent().setKey(string(_key));
    return _addSubObject(newObj);  // !could change the item order!
}

DataObject& DataObject::operator[](std::string&& _key)
{
    _assert(m_type == DataType::NotInitialized || m_type == DataType::Object,
        "m_type == DataType::NotInitialized || m_type == DataType::Object (DataObject& operator[])");

    if (m_subObjectKeys.count(_key))
        return m_subObjectKeys.at(_key).getContent();

    spDataObject newObj(new DataObject(DataType::NotInitialized));
    newObj.getContent().setKey(std::forward<string&&>(_key));
    return _addSubObject(newObj);  // !could change the item order!
}

DataObject& DataObject::operator=(std::string&& _value)
{
    _assert(m_type == DataType::String || m_type == DataType::NotInitialized,
        "In DataObject=(string) DataObject must be string or NotInitialized!");
    m_type = DataType::String;
    m_value = std::move(_value);
    return *this;
}

DataObject& DataObject::operator=(std::string const& _value)
{
    _assert(m_type == DataType::String || m_type == DataType::NotInitialized,
        "In DataObject=(string) DataObject must be string or NotInitialized!");
    m_type = DataType::String;
    m_value = _value;
    return *this;
}

spDataObjectMove dataobject::move(spDataObject& _obj)
{
    spDataObjectMove m;
    m.assignPointer(_obj);
    _obj.null();
    return m;
}
