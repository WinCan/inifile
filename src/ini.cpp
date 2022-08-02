#include "inifile/ini.h"
#include <fstream>
#include <algorithm>

namespace inifile {
template <typename T>
std::basic_string<T> view_to_string(std::basic_string_view<T> sv)
{
    return std::basic_string<T>(sv.data(), sv.length());
}

template <typename T, typename V>
bool map_contains(const std::map<std::basic_string<T>, V>& map, std::basic_string_view<T> sv)
{
    return std::find_if(map.cbegin(), map.cend(), [sv](auto it){
        return it.first == sv;
    }) != map.end();
}

template<typename CharT>
file<CharT>::file(const CharType* str)
    : _path(str)
{}

template<typename CharT>
file<CharT>::file(const StrType& str)
    : _path(str)
{}

template<typename CharT>
file<CharT>::~file()
{
    if(_writeOnClose)
        file<CharT>::write();
}

template<typename CharT>
const file<CharT>::Groups& file<CharT>::groups() const
{
    return _structure;
}

template<typename CharT>
file<CharT>::Groups& file<CharT>::groups()
{
    return _structure;
}

template<typename CharT>
file<CharT>::Values& file<CharT>::group(StrViewType str)
{
    return _structure[view_to_string(str)];
}

template<typename CharT>
const file<CharT>::Values& file<CharT>::group(StrViewType str) const
{
    if (str.empty() || !map_contains(_structure, str))
    {
        // General by default??
        return EMPTY_GROUP;
    }
    return _structure.at(_currentGroup);
}

template<typename CharT>
std::vector<typename file<CharT>::StrType> file<CharT>::groupNames() const
{
    std::vector<StrType> names(_structure.size());
    for (auto gIt = _structure.cbegin(); gIt != _structure.cend(); ++gIt)
        names.push_back(gIt->first);
    return names;
}

template<typename CharT>
bool file<CharT>::contains(StrViewType str) const
{
    if (!_currentGroup.empty() && !_structure.contains(_currentGroup))
        return false;
    auto groupName = _currentGroup.empty() ? groupFromKey(str) : _currentGroup;
    auto name = _currentGroup.empty() ? nameFromKey(str) : str;
    if(groupName.empty() || !map_contains(_structure, groupName))
        return false;
    if (name.empty())
        return true;
    const auto& group = _structure.at(view_to_string(groupName));
    return map_contains(group, name);
}

template<typename CharT>
void file<CharT>::add(const Values &values)
{
    for(auto it = values.cbegin() ; it != values.cend() ; ++it)
        value(it->first) = it->second;
}

template<typename CharT>
const file<CharT>::Values& file<CharT>::values() const
{
    if (_currentGroup.empty() || !_structure.contains(_currentGroup))
    {
        // General by default??
        return EMPTY_GROUP;
    }
    return _structure.at(_currentGroup);
}

template<typename CharT>
file<CharT>::Values& file<CharT>::values()
{
    return _structure[_currentGroup];
}

template<typename CharT>
const file<CharT>::StrType& file<CharT>::value(StrViewType str) const
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

template<typename CharT>
file<CharT>::StrType& file<CharT>::value(StrViewType str)
{
    auto groupName = _currentGroup.empty() ? view_to_string(groupFromKey(str)) : _currentGroup;
    auto name = view_to_string(_currentGroup.empty() ? nameFromKey(str) : str);
    auto& group = _structure[groupName];
    return group[name];
}

template<typename CharT>
void file<CharT>::beginGroup(StrViewType str)
{
    _currentGroup = str;
}

template<typename CharT>
void file<CharT>::endGroup()
{
    _currentGroup.clear();
}

template<typename CharT>
file<CharT>::StrViewType file<CharT>::groupFromKey(StrViewType str)
{
    auto it = str.find(CharType('.'));
    if (it == StrType::npos)
        return str;
    return str.substr(0, it);
}

template<typename CharT>
file<CharT>::StrViewType file<CharT>::nameFromKey(StrViewType str)
{
    auto it = str.find(CharType('.'));
    if (it == StrType::npos)
        return {};
    return str.substr(it + 1, StrType::npos);
}

template<typename CharT>
bool file<CharT>::read()
{
    _structure.clear();
    _currentGroup.clear();
    std::basic_ifstream<CharType> file(_path.c_str());
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

template<typename CharT>
bool file<CharT>::write()
{
    _currentGroup.clear();
    std::basic_ofstream<CharType> file(_path.c_str(), std::ios_base::out);
    if (!file.is_open())
        return false;
    if constexpr(!std::is_same_v<CharType, char>)
        file.imbue(std::locale(std::locale::classic(), new std::codecvt_utf8<CharType>));
    for (auto gIt = _structure.cbegin(); gIt != _structure.cend(); ++gIt)
    {
        file << '[' << gIt->first << ']' << '\n';
        for (auto vIt = gIt->second.cbegin(); vIt != gIt->second.cend(); ++vIt)
            file << vIt->first << '=' << vIt->second << '\n';
    }
    return true;
}

template<typename CharT>
file<CharT>::Values& file<CharT>::operator[](StrViewType key)
{
    return group(key);
}

template<typename CharT>
void file<CharT>::setWriteOnClose(bool val)
{
    _writeOnClose = val;
}

template<typename CharT>
ParsedValue<typename file<CharT>::StrViewType> file<CharT>::parseLine(StrViewType line)
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

template<typename CharT>
void file<CharT>::trim(StrViewType &value)
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

#if (defined(_WIN32) && !defined(USE_NARROW_ONLY))
template class inifile::file<wchar_t>;
#endif
template class inifile::file<char>;