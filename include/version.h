#pragma once

#include <string>
#include <vector>

std::string getVersion();
std::string getBuildStamp();
std::string getAllInfo(int argc, char **argv, const std::string &progname);

int handleVersionOption(int argc, char **argv, const char *progname);

char *      argv2cmdline(int argc, char *argv[]);
std::string argv2cmdline(std::vector<char *>, bool progname = true);
char *      VERuser();
char *      VERfileTimeStamp(char *fname);
char *      VERcurTimeStamp(void);
