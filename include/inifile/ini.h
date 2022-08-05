#pragma once

#include <filesystem>
#include <map>
#include <vector>

namespace inifile{
    enum class ValueType
    {
        Group,
        Comment,
        Value,
        Unknown
    };
    
    template<typename StrViewType>
    struct ParsedValue
    {
        ValueType type = ValueType::Unknown;
        StrViewType key;
        StrViewType value;
    };

    template<typename CharT=char>
    class file
    {
        using CharType = CharT;
        using StrType = std::basic_string<CharType>;
        using StrViewType = std::basic_string_view<CharType>;
        using Values = std::map<StrType, StrType>;
        using Groups = std::map<StrType, Values>;

        public:
            file(const std::filesystem::path& str);
            virtual ~file();
            
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
            
            void beginGroup(StrViewType str);
            void endGroup();
            
            bool read();
            bool write();

            Values& operator[](StrViewType key);
            void setWriteOnClose(bool val);

        private:
            file::Values EMPTY_GROUP;
            file::StrType EMPTY_VALUE;
            static StrViewType groupFromKey(StrViewType str);
            static StrViewType nameFromKey(StrViewType str);
        
            ParsedValue<StrViewType> parseLine(StrViewType line);
            static void trim(StrViewType& value);
            std::filesystem::path _path;
            StrType _currentGroup;
            Groups _structure;
            bool _writeOnClose = true;
    };
}
