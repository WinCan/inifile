#pragma once

#include "ini.h"

#include <filesystem>
#include <fstream>

namespace inifile {
template <typename CharType>
struct FileIOHandler : public inifile::IOHandler<CharType>
{
    using Base = inifile::IOHandler<CharType>;
    explicit FileIOHandler(const std::filesystem::path& path, FlowDirection direction = FlowDirection::In)
        : _mode(direction == FlowDirection::In ? std::fstream::in : std::fstream::out)
    {
        _stream = std::basic_fstream<CharType>(path.c_str(), _mode);
    }
    virtual ~FileIOHandler() = default;
    bool read_line(typename Base::StrType& line) override
    {
        return (_mode & std::fstream::in) && _stream.is_open() && std::getline(_stream, line);
    }
    bool write_line(typename Base::StrViewType line) override
    {
        if (!(_mode & std::fstream::out) || !_stream.is_open())
            return false;
        _stream << line << '\n';
        return _stream.good();
    }
    std::basic_fstream<char> _stream;
    std::fstream::openmode _mode = std::fstream::in;
};
} // namespace inifile