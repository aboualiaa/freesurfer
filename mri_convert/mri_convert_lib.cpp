//
// Created by Ahmed Abou-Aliaa on 29.11.19.
//

#include "mri_convert_lib.hpp"

int returnAnInt(int toReturn) {
  return toReturn;
}

bool usage_message_lib(FILE *stream) {

  if (stream != nullptr) {
    fmt::fprintf(stream, "\n");
    fmt::fprintf(stream, "type -u for usage\n");
    fmt::fprintf(stream, "\n");
    return true;
  }

  return false;
}
