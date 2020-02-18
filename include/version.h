#pragma once

#include <string>
#include<gsl/multi_span>

std::string getVersion();
std::string getBuildStamp();
std::string getAllInfo(int argc, char **argv, const std::string& progname);

int handleVersionOption(int argc, char** argv, const char* progname);

char *argv2cmdline(int argc, char *argv[]);
std::string argv2cmdline(gsl::multi_span<char *>, bool progname = true);
char *VERuser();
char *VERfileTimeStamp(char *fname);
char *VERcurTimeStamp(void);
