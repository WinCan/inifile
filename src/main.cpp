#include "inifile/file_handler.h"
#include "inifile/strings_handler.h"
#include "inifile/ini.h"

#include <cassert>


// Example use and test cases

int main()
{
    // Handle read/write operations for a string vector interface
    inifile::StringsIOHandler<char> handler(32, inifile::FlowDirection::Out);
#if (defined(_WIN32) && !defined(USE_NARROW_ONLY))
    {
        // Handle read/write operations and pass them to file
        FileIOHandler<wchar_t> handler(std::filesystem::path("file.conf"));
        // inifile::file<wchar_t>
        inifile::file<wchar_t> f(&handler);
        // Add value to group
        f.beginGroup(L"MyGroupąą");
        f.value(L"Jakąś") = L"Kóźnia";
        f.value(L"Inny") = L"pfff";
        f.endGroup();
        // Add single value to group
        f.value(L"AnotherGroup.Value1") = L"33";
        // Using bracket operator
        f[L"AnotherGroup"][L"Value2"] = L"36";
        // Add multiple values in different groups
        f.add({{L"first.val1", L"42"}, {L"second.val2", L"33.12"}});
        // Disable auto flush on scope exit, can be done if handler is not
        // set but rather used as read/write argument
        // f.setWriteOnClose(false);
    }
#endif
    {
        // Handle read/write operations for a file interface
        // inifile::FileIOHandler<char> handler(std::filesystem::path("file.conf"), inifile::FlowDirection::Out);
        // inifile::file<char>
        inifile::file<char> f(&handler);
        // Add value to group
        f.beginGroup("MyGroupąą");
        f.value("Jakąś") = "Kóźnia";
        f.value("Inny") = "pfff";
        f.endGroup();
        // Add single value to group
        f.value("AnotherGroup.Value1") = "33";
        // Using bracket operator
        f["AnotherGroup"]["Value2"] = "36";
        // Add multiple values in different groups
        f.add({{"first.val1", "42"}, {"second.val2", "33.12"}});
        // Disable auto flush on scope exit, can be done if handler is not
        // set but rather used as read/write argument
        // f.setWriteOnClose(false);
    }
    {
        // Handle read/write operations and pass them to file
        // inifile::FileIOHandler<char> handler(std::filesystem::path("file.conf"), inifile::FlowDirection::In);
        // inifile::file<char>
        // This is in memory handler so reopen will change mode and reset internal data pointer
        handler.reopen(inifile::FlowDirection::In);
        inifile::file f;
        // explicit load using custom handler
        assert(f.read(&handler));
        // Value in group
        assert(f.value("AnotherGroup.Value1") == "33");
        // Check group
        assert(f.contains("AnotherGroup"));
        // Check value in group
        assert(f.contains("AnotherGroup.Value1"));
        // No group, no value in group, no value in non existing group
        assert(!f.contains("NonExisting"));
        assert(!f.contains("AnotherGroup.Value3"));
        assert(!f.contains("NonExisting.Value3"));
        // Get all values from group
        auto vals = f.group("MyGroupąą");
        assert(vals.size() == 2);
        // Group scope
        f.beginGroup("first");
        assert(!f.contains("AnotherGroup"));
        assert(!f.contains("first"));
        // Still group scope
        assert(f.contains("val1"));
        assert(f.value("val1") == "42");
        // This is global scope
        assert(f["AnotherGroup"]["Value2"] == "36");
        // Can get group
        auto group = f.group("first");
        assert(group["val1"] == "42");
        const auto cf = f;
        const auto cgroup = cf.group("first");
        // This will fail because cf and cgroup is const and [] will create group/value if there isn't one yet
        // assert(cgroup["val1"] == "42");
        // This might throw exception if there is no such key, so it is good to check cgroup.contains("val1")
        assert(cgroup.at("val1") == "42");
        f.endGroup();
    }
    return 0;
}