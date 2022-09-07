#include "inifile/ini.h"
#include <algorithm>
#include <codecvt>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>

namespace inifile {
template <typename T>
std::basic_string<T> view_to_string(std::basic_string_view<T> sv)
{
    return std::basic_string<T>(sv.data(), sv.length());
}

template <typename T, typename V>
bool map_contains(const std::map<std::basic_string<T>, V>& map, std::basic_string_view<T> sv)
{
    return std::find_if(map.cbegin(), map.cend(), [sv](auto it) { return it.first == sv; }) != map.end();
}

template <typename CharT>
file<CharT>::file(IOHandler<CharT>* handler)
    : _handler(handler)
{}

template <typename CharT>
file<CharT>::~file()
{
    if (_writeOnClose && _handler)
        file<CharT>::write(_handler);
}

template <typename CharT>
void file<CharT>::setIOHandler(IOHandler<CharType>* handler)
{
    _handler = handler;
}

template <typename CharT>
const typename file<CharT>::Groups& file<CharT>::groups() const
{
    return _structure;
}

template <typename CharT>
typename file<CharT>::Groups& file<CharT>::groups()
{
    return _structure;
}

template <typename CharT>
typename file<CharT>::Values& file<CharT>::group(StrViewType str)
{
    return _structure[view_to_string(str)];
}

template <typename CharT>
const typename file<CharT>::Values& file<CharT>::group(StrViewType str) const
{
    if (str.empty() || !map_contains(_structure, str))
    {
        // General by default??
        return EMPTY_GROUP;
    }
    return _structure.at(_currentGroup);
}

template <typename CharT>
std::vector<typename file<CharT>::StrType> file<CharT>::groupNames() const
{
    std::vector<StrType> names(_structure.size());
    for (auto gIt = _structure.cbegin(); gIt != _structure.cend(); ++gIt)
        names.push_back(gIt->first);
    return names;
}

template <typename CharT>
bool file<CharT>::contains(StrViewType str) const
{
    if (!_currentGroup.empty() && !_structure.contains(_currentGroup))
        return false;
    auto groupName = _currentGroup.empty() ? groupFromKey(str) : _currentGroup;
    auto name = _currentGroup.empty() ? nameFromKey(str) : str;
    if (groupName.empty() || !map_contains(_structure, groupName))
        return false;
    if (name.empty())
        return true;
    const auto& group = _structure.at(view_to_string(groupName));
    return map_contains(group, name);
}

template <typename CharT>
void file<CharT>::add(const Values& values)
{
    for (auto it = values.cbegin(); it != values.cend(); ++it)
        value(it->first) = it->second;
}

template <typename CharT>
const typename file<CharT>::Values& file<CharT>::values() const
{
    if (_currentGroup.empty() || !_structure.contains(_currentGroup))
    {
        // General by default??
        return EMPTY_GROUP;
    }
    return _structure.at(_currentGroup);
}

template <typename CharT>
typename file<CharT>::Values& file<CharT>::values()
{
    return _structure[_currentGroup];
}

template <typename CharT>
const typename file<CharT>::StrType& file<CharT>::value(StrViewType str) const
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

template <typename CharT>
typename file<CharT>::StrType& file<CharT>::value(StrViewType str)
{
    auto groupName = _currentGroup.empty() ? view_to_string(groupFromKey(str)) : _currentGroup;
    auto name = view_to_string(_currentGroup.empty() ? nameFromKey(str) : str);
    auto& group = _structure[groupName];
    return group[name];
}

template <typename CharT>
typename file<CharT>::StrType file<CharT>::key(const StrType& group, const StrType& name) const
{
    return group + StrType(".") + name;
}

template <typename CharT>
void file<CharT>::beginGroup(StrViewType str)
{
    _currentGroup = str;
}

template <typename CharT>
void file<CharT>::endGroup()
{
    _currentGroup.clear();
}

template<typename CharT>
typename file<CharT>::StrType file<CharT>::currentGroup() const
{
    return _currentGroup;
}

template <typename CharT>
typename file<CharT>::StrViewType file<CharT>::groupFromKey(StrViewType str)
{
    auto it = str.find(CharType('.'));
    if (it == StrType::npos)
        return str;
    return str.substr(0, it);
}

template <typename CharT>
typename file<CharT>::StrViewType file<CharT>::nameFromKey(StrViewType str)
{
    auto it = str.find(CharType('.'));
    if (it == StrType::npos)
        return {};
    return str.substr(it + 1, StrType::npos);
}

template <typename CharT>
bool file<CharT>::read(IOHandler<CharType>* handler)
{
    _structure.clear();
    _currentGroup.clear();

    if (handler == nullptr && _handler == nullptr)
        return false;
    if (handler == nullptr)
        handler = _handler;

    StrType line;
    Values* vals = nullptr;
    while (handler->read_line(line))
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
    return true;
}

template <typename CharT, typename StringT = std::basic_string<CharT>>
StringT get_string(std::basic_ostringstream<CharT>& stream)
{
    auto str = stream.str();
    stream.clear();
    stream.str(StringT());
    return str;
}

template <typename CharT>
bool file<CharT>::write(IOHandler<CharType>* handler)
{
    _currentGroup.clear();

    if (handler == nullptr && _handler == nullptr)
        return false;
    if (handler == nullptr)
        handler = _handler;

    std::basic_ostringstream<CharType> stream;
    if constexpr (!std::is_same_v<CharType, char>)
        stream.imbue(std::locale(std::locale::classic(), new std::codecvt_utf8<CharType>));
    for (auto gIt = _structure.cbegin(); gIt != _structure.cend(); ++gIt)
    {
        stream << '[' << gIt->first << ']';
        if(!handler->write_line(get_string(stream)))
            return false;
        for (auto vIt = gIt->second.cbegin(); vIt != gIt->second.cend(); ++vIt)
        {
            stream << vIt->first << '=' << vIt->second;
            if(!handler->write_line(get_string(stream)))
                return false;
        }
    }
    return true;
}

template <typename CharT>
typename file<CharT>::Values& file<CharT>::operator[](StrViewType key)
{
    return group(key);
}

template <typename CharT>
void file<CharT>::setWriteOnClose(bool val)
{
    _writeOnClose = val;
}

template <typename CharT>
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

template <typename CharT>
void file<CharT>::trim(StrViewType& value)
{
    auto len = value.length();
    auto start = 0;
    if (value.ends_with(CharType('"')))
        --len;
    if (value.starts_with(CharType('"')))
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