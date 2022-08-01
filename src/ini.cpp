#include "inifile/ini.h"
#include <fstream>
#include <algorithm>

namespace inifile {
Values EMPTY_GROUP;
StrType EMPTY_VALUE;

StrType view_to_string(StrViewType sv)
{
    return StrType(sv.data(), sv.length());
}

template <typename T>
bool map_contains(const std::map<std::string, T>& map, StrViewType sv)
{
    return std::find_if(map.cbegin(), map.cend(), [sv](auto it){
        return it.first == sv;
    }) != map.end();
}

file::file(const CharType* str)
    : _path(str)
{}

file::file(const StrType& str)
    : _path(str)
{}

const Groups& file::groups() const
{
    return _structure;
}

Groups& file::groups()
{
    return _structure;
}

Values& file::group(StrViewType str)
{
    return _structure[view_to_string(str)];
}

const Values& file::group(StrViewType str) const
{
    if (str.empty() || !map_contains(_structure, str))
    {
        // General by default??
        return EMPTY_GROUP;
    }
    return _structure.at(_currentGroup);
}

std::vector<StrType> file::groupNames() const
{
    std::vector<StrType> names(_structure.size());
    for (auto gIt = _structure.cbegin(); gIt != _structure.cend(); ++gIt)
        names.push_back(gIt->first);
    return names;
}

bool file::contains(StrViewType str) const
{
    if (!_currentGroup.empty() && !_structure.contains(_currentGroup))
        return false;
    auto groupName = _currentGroup.empty() ? groupFromKey(str) : _currentGroup;
    auto name = _currentGroup.empty() ? nameFromKey(str) : str;
    if(groupName.empty() || !map_contains(_structure, groupName))
        return false;
    const auto& group = _structure.at(view_to_string(groupName));
    return map_contains(group, str);
}

const Values& file::values() const
{
    if (_currentGroup.empty() || !_structure.contains(_currentGroup))
    {
        // General by default??
        return EMPTY_GROUP;
    }
    return _structure.at(_currentGroup);
}

Values& file::values()
{
    return _structure[_currentGroup];
}

const StrType& file::value(StrViewType str) const
{
    auto groupName = _currentGroup.empty() ? groupFromKey(str) : _currentGroup;
    if (groupName.empty() || !map_contains(_structure, groupName))
        return EMPTY_VALUE;
    auto name = _currentGroup.empty() ? nameFromKey(str) : str;
    const auto& group = _structure.at(view_to_string(groupName));
    if (name.empty() || !map_contains(group, name))
        return EMPTY_VALUE;
    return group.at(view_to_string(name));
}

StrType& file::value(StrViewType str)
{
    auto groupName = _currentGroup.empty() ? view_to_string(groupFromKey(str)) : _currentGroup;
    auto name = view_to_string(_currentGroup.empty() ? nameFromKey(str) : str);
    auto& group = _structure[groupName];
    return group[name];
}

void file::beginGroup(StrViewType str)
{
    _currentGroup = str;
}

void file::endGroup()
{
    _currentGroup.clear();
}

StrViewType file::groupFromKey(StrViewType str)
{
    auto it = str.find(CharType('.'));
    if (it == StrType::npos)
        return {};
    return str.substr(0, it);
}

StrViewType file::nameFromKey(StrViewType str)
{
    auto it = str.find(CharType('.'));
    if (it == StrType::npos)
        return {};
    return str.substr(it + 1, StrType::npos);
}

bool file::read()
{
    _structure.clear();
    _currentGroup.clear();
    std::ifstream file(_path.c_str());
    if (!file.is_open())
        return false;
    StrType line;
    Values* vals = nullptr;
    while (std::getline(file, line))
    {
        auto keyVal = parseLine(line);
        if (keyVal.type == ValueType::Comment || keyVal.type == ValueType::Unknown)
            continue;

        if (keyVal.type == ValueType::Group)
        {
            vals = &group(keyVal.key);
            continue;
        }

        if (vals == nullptr)
            continue;

        (*vals)[view_to_string(keyVal.key)] = view_to_string(keyVal.value);
    }
    file.close();
    return true;
}

bool file::write()
{
    _currentGroup.clear();
    std::ofstream file(_path.c_str());
    if (!file.is_open())
        return false;
    for (auto gIt = _structure.cbegin(); gIt != _structure.cend(); ++gIt)
    {
        file << '[' << gIt->first << ']' << '\n';
        for (auto vIt = gIt->second.cbegin(); vIt != gIt->second.cend(); ++vIt)
        {
            file << vIt->first << '=' << vIt->second << '\n';
        }
    }
    return true;
}

Values& file::operator[](StrViewType key)
{
    return group(key);
}

ParsedValue file::parseLine(StrViewType line)
{
    // naive, no trimming for group and comment lines
    if (line.starts_with(CharType(';')) || line.starts_with(CharType('#')))
        return {ValueType::Comment};
    if (line.starts_with(CharType('[')) && line.ends_with(CharType(']')))
        return {ValueType::Group, line.substr(1, line.size() - 2)};
    auto it = line.find(CharType('='));
    if (it == StrType::npos || it == line.size())
        return {ValueType::Unknown};
    auto key = line.substr(0, it);
    auto value = line.substr(it + 1, StrType::npos);
    trim(key);
    trim(value);
    return {ValueType::Value, key, value};
}

void file::trim(StrViewType &value)
{
    auto len = value.length();
    auto start = 0;
    if(value.ends_with(CharType('"')))
        --len;
    if(value.starts_with(CharType('"')))
    {
        --len;
        start = 1;
    }
    value = value.substr(start, len);
}

} // namespace inifile
