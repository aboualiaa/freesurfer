#ifndef LUT_H
#define LUT_H

#include <map>
#include <string>
#include <vector>

class LookupTable {

  struct LabelInfo {
    bool        valid;
    std::string name;
  };

public:
  LookupTable(){};
  LookupTable(std::string filename);

  void add(int label, std::string labelname = "");
  bool importFromFile(std::string filename);

  std::vector<int> labels();

  bool hasNameInfo();
  bool hasColorInfo();
  bool empty();

  // extensions of std::map
  LabelInfo &operator[](int index) { return labelmap[index]; }
  using iterator       = std::map<int, LabelInfo>::iterator;
  using const_iterator = std::map<int, LabelInfo>::const_iterator;
  iterator begin() { return labelmap.begin(); }
  iterator end() { return labelmap.end(); }

private:
  std::map<int, LabelInfo> labelmap;
};

#endif
