#pragma once

#include <map>
#include <string>
#include <vector>

namespace inifile {
enum class FlowDirection{
    In,
    Out,
};

template <typename CharType>
struct IOHandler
{
    typedef typename std::basic_string<CharType> StrType;
    typedef typename std::basic_string_view<CharType> StrViewType;
    virtual bool read_line(StrType& line) = 0;
    virtual bool write_line(StrViewType line) = 0;
    virtual bool open_device(FlowDirection open_mode = FlowDirection::In) = 0;
    virtual bool close_device() = 0;
    virtual bool is_opened(FlowDirection open_mode) = 0;
    virtual ~IOHandler() = default;
};

enum class ValueType
{
    Group,
    Comment,
    Value,
    Unknown
};

template <typename StrViewType>
struct ParsedValue
{
    ValueType type = ValueType::Unknown;
    StrViewType key;
    StrViewType value;
};

template <typename CharT = char>
class file
{
    using CharType = CharT;
    using StrType = std::basic_string<CharType>;
    using StrViewType = std::basic_string_view<CharType>;
    using Values = std::map<StrType, StrType>;
    using Groups = std::map<StrType, Values>;

public:
    file(IOHandler<CharT>* handler = nullptr);
    virtual ~file();

    void setIOHandler(IOHandler<CharType>* handler);

    const Groups& groups() const;
    Groups& groups();
    Values& group(StrViewType str);
    const Values& group(StrViewType str) const;
    std::vector<StrType> groupNames() const;
    bool contains(StrViewType str) const;
    void add(const Values& values);

    const Values& values() const;
    Values& values();
    const StrType& value(StrViewType str) const;
    StrType& value(StrViewType str);
    StrType key(const StrType& group, const StrType& name) const;

    void beginGroup(StrViewType str);
    void endGroup();
    StrType currentGroup() const;

    bool read(IOHandler<CharType>* handler = nullptr);
    bool write(IOHandler<CharType>* handler = nullptr);

    Values& operator[](StrViewType key);
    void setWriteOnClose(bool val);

private:
    file::Values EMPTY_GROUP;
    file::StrType EMPTY_VALUE;
    static StrViewType groupFromKey(StrViewType str);
    static StrViewType nameFromKey(StrViewType str);

    ParsedValue<StrViewType> parseLine(StrViewType line);
    static void trim(StrViewType& value);
    StrType _currentGroup;
    Groups _structure;
    IOHandler<CharType>* _handler = nullptr;
    bool _writeOnClose = true;
};
} // namespace inifile
