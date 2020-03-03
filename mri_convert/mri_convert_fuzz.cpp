//
// Created by Ahmed Abou-Aliaa on 27.02.20.
//

#include "mri_convert_lib.hpp"

#include <iostream>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  std::vector<std::string> wow(20, (char *)Data);
  std::string              hello{(char *)Data};
  std::cout << wow[1] << std::endl;
  fs::util::cli::check_string_values(wow, hello, hello);
  return 0; // Non-zero return values are reserved for future use.
}
