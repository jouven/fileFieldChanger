fileFieldChanger
================

Commandline program to change config files values

/*
* commandline program
* can be used to modify config files like the one for ssh (sshd_config)
* the program will attempt to modify the config values i.e Port 22 --> Port 2222
* the expected arguments are:
* -p "pathofthefile"
* -c "commentcharacter" //i.e "#" in the case of sshd_config
* -s "separator character" //i.e Port 2222 --> the separator is a whitespace
* -v //verbose
* -a "config1" "value1" //can be used multiple times, one pair of key value i.e: Port 5555
* remember to wrap you values in quotes if there are white spaces so they don't get ignored
* the main will return 0 if it hasn't managed to change anything and 1 otherwise
* it has some error control for missing arguments but don't expect much
* it will abort if it finds invalid UTF-8 encoding
* libraries required: GCC version that supports C++11, BOOST and UTF8-CPP
* */
