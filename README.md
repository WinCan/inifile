# inifile

Simple library for ini file read/write. Can be used for char and wchar_t (on Windows platform USE_NARROW_ONLY option).
Supports:
* Comments (; and #)
* Scopes/groups (no nested groups)
* Value trimming (white characters and '/")
* Key can be used as group.value_name, no need to use [][] syntax, although it is possible
* Values are stored as char/wchar_t type so no conversion is done on the fly (use your own wrapper)


## Further work
I have not done any benchmarks yet, so feel free to do so. I have no plans (yet) for making it header only although it can be done easily.

## Tutorial
Tutorial is available in the test file - main.cpp. 

## License
Licensed under BSD license. 
