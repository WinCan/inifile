#pragma once

#include <filesystem>
#include <map>
#include <vector>

namespace inifile{
    using CharType = std::filesystem::path::value_type;
    using StrType = std::filesystem::path::string_type;
    using StrViewType = std::basic_string_view<std::filesystem::path::value_type>;

    using Values = std::map<StrType, StrType>;
    using Groups = std::map<StrType, Values>;

    enum class ValueType
    {
        Group,
        Comment,
        Value,
        Unknown
    };
    
    struct ParsedValue
    {
        ValueType type = ValueType::Unknown;
        StrViewType key;
        StrViewType value;
    };

    class file
    {
        public:
            file(const CharType* str);
            file(const StrType& str);
            virtual ~file() = default;
            
            const Groups& groups() const;
            Groups& groups();
            Values& group(StrViewType str);
            const Values& group(StrViewType str) const;
            std::vector<StrType> groupNames() const;
            bool contains(StrViewType str) const;
            
            const Values& values() const;
            Values& values();
            const StrType& value(StrViewType str) const;
            StrType& value(StrViewType str);
            
            void beginGroup(StrViewType str);
            void endGroup();
            
            bool read();
            bool write();

            Values& operator[](StrViewType key);

        private:
            static StrViewType groupFromKey(StrViewType str);
            static StrViewType nameFromKey(StrViewType str);
        
            ParsedValue parseLine(StrViewType line);
            static void trim(StrViewType& value);
            std::filesystem::path _path;
            StrType _currentGroup;
            Groups _structure;
    };
}