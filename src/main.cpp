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
 * right now it always verbose the arguments passed and ignored commented lines
 * it will abort if it finds invalid UTF-8 encoding
 * libraries required: GCC version that supports C++11, BOOST and UTF8-CPP
 * */

#include <iostream>
#include <fstream>
#include "utf8.h"
#include "boost/algorithm/string.hpp"
#include <map>

typedef std::pair<std::string, std::string> pairstrstr;

int main(int argc, char** argv)
{
	//dictionary of parameters
	std::map<std::string, std::string> params;

	std::string filepath;
	std::string commentchar;
	std::string separatorchar;
	bool verbose = false;
	//check the arguments
	for (auto i = 1; i < argc; i++)
	{
		std::string argtmp(argv[i]);
		if (i + 1 < argc && argv[i + 1] != nullptr)
		{
			if (argtmp == "-p")
			{
				filepath = std::string(argv[i + 1]);
				continue;

			}
			if (argtmp == "-c")
			{
				commentchar = std::string(argv[i + 1]);
				continue;
			}
			if (argtmp == "-s")
			{
				separatorchar = std::string(argv[i + 1]);
				continue;
			}
			if (argtmp == "-v")
			{
				verbose = true;
			}
			if (argtmp == "-a")
			{
				//insert the pair into the params dictionary
				if (i + 2 < argc && argv[i + 2] != nullptr)
				{
					params.insert(
							pairstrstr(std::string(argv[i + 1]),
									std::string(argv[i + 2])));
				}
			}
		}
	}
	//verbose arguments information
	if (verbose)
	{
		std::cout << " argc " << argc << std::endl;
		for (auto i = params.begin(); i != params.end(); i = std::next(i))
		{
			std::cout << "key " + i->first << " value " << i->second
					<< std::endl;
		}
	}
	//for debug purposes
	//params.clear();
	//params.insert(pairstrstr("-p", "sshd_config"));
	//params.insert(pairstrstr("-c", "#"));
	//params.insert(pairstrstr("-s", " "));
	//params.insert(pairstrstr("-a",R"("AllowUsers jou@127.0.0.1")"));

	//check that all the necessary arguments have been passed
	if (filepath.empty())
	{
		std::cout << u8R"(No path "-p" parameter specified)" << std::endl;
		return 0;
	}

	if (commentchar.empty())
	{
		std::cout << u8R"(No comment character "-c" parameter specified, this parameter might need to be wrapped in quote characters)" << std::endl;
		return 0;
	}

	if (separatorchar.empty())
	{
		std::cout << u8R"(No separator character "-s" parameter specified, this parameter might need to be wrapped in quote characters)" << std::endl;
		return 0;
	}

	//open the file
	std::ifstream file(filepath);
	if (!file.is_open())
	{
		std::cout << "Could not open " << filepath << std::endl;
		return 0;
	}
	//create the new modified temporal file
	std::ofstream newFile(filepath + ".tmp");
	//line counter
	unsigned int line_count = 0;
	//string to get the lines with getline
	std::string line;
	//"global" variable to control if a change has happened
	int changeg = 0;
	//for every line of the text file do a getline
	while (std::getline(file, line))
	{
		line_count++;
		//if the line is empty put a \n in the new file
		if (line.empty())
		{
			newFile.put('\n');
			continue;
		}
		//check if there is any invalid utf8 encoding in the line
		auto end_it = utf8::find_invalid(line.begin(), line.end());
		if (end_it != line.end())
		{
			std::cout << "Invalid UTF-8 encoding detected at line "
					<< line_count << "\n";
			std::cout << "This part is fine: "
					<< std::string(line.begin(), end_it) << "\n";
			std::cout << "Quitting...";
			std::remove((filepath + ".tmp").data());
			break;
		}
		//bool to control if the line is just a comment
		auto commentedline = false;
		//iterate the characters of the line until it finds a non whitespace-comment character
		for (auto it = line.begin(); it != line.end(); it = std::next(it))
		{
			//ignore trailing spaces
			if (*it == ' ')
			{
				continue;
			}
			//if it finds a comment character take notice
			if (*it == commentchar.front())
			{
				if (verbose)
				{
					std::cout << "Commented line " << line_count
							<< " ignoring line" << std::endl;
				}
				commentedline = true;
				break;
			}
			break;
		}

		if (commentedline)
		{
			//if a comment character was found copy the line to the new file and continue to the next one
			newFile.write(line.data(), line.length()).put('\n');
			continue;
		}
		//split the line in a vector with the comment character
		//first element == none comment, the config data
		//rest of elements == comments
		std::vector<std::string> stringvectorcomm;
		boost::algorithm::split(stringvectorcomm, line,
				boost::algorithm::is_any_of(commentchar),
				boost::algorithm::token_compress_on);
		//split the first element of stringvectorcomm with the separator character
		//first element == key
		//second element == value , even if the value has spaces-"more separator characters within" doesn't matter because it discards the rest
		std::vector<std::string> stringvectorsep;
		boost::algorithm::split(stringvectorsep, stringvectorcomm.front(),
				boost::algorithm::is_any_of(separatorchar),
				boost::algorithm::token_compress_on);
		//iterate the vector with the config data trying to find a config with some key we want to change
		auto itstr = stringvectorsep.begin();
		bool change = false;
		while (itstr != stringvectorsep.end())
		{
			//iterate the params in the dictionary
			for (auto itarg = params.begin(); itarg != params.end(); itarg =
					std::next(itarg))
			{
				//if the current one matches
				if (*itstr == itarg->first)
				{
					//get the next, which is the value we want to change
					itstr = std::next(itstr);
					if (itstr != stringvectorsep.end())
					{
						//change it
						*itstr = itarg->second;
						//take notice
						change = true;
						//if a changed happened skip the rest of the dictionary
						break;
					}
				}
			}
			//if a changed happened skip the rest of the line config data
			if (change)
			{
				break;
			}
			itstr = std::next(itstr);
		}
		//the modified line that will go into the "new" file
		std::string linestrtmp;
		//if a change happened
		if (change)
		{
			//get the config data until the change and put it on the modified line, linestrtmp, variable
			for (auto ittmp = stringvectorsep.begin(); ittmp != itstr; ittmp =
					std::next(ittmp))
			{
				if (!(*ittmp).empty())
				{
					linestrtmp += *ittmp + separatorchar;
				}
			}
			//assign one last time, the change itself, because ittmp != itstr skips the last assignment which contains the config data that it changed
			linestrtmp += *itstr;
			changeg = 1;
			//add the comments part of the line if it had comments
			if (stringvectorcomm.size() > 1)
			{
				for (auto ittmp = std::next(stringvectorcomm.begin());
						ittmp != stringvectorcomm.end();
						ittmp = std::next(ittmp))
				{
					linestrtmp += commentchar + *ittmp;
				}
			}
		}
		else
		{
			linestrtmp = line;
		}
		//write the line in the new temporal file
		newFile.write(linestrtmp.data(), linestrtmp.length()).put('\n');
	}
	//close both files
	file.close();
	newFile.close();
	//remove the original file
	std::remove(filepath.data());
	//rename the temporal file to the original file
	std::rename((filepath + ".tmp").data(), filepath.data());
	return changeg;
}
