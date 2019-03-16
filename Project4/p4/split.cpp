#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>
#include <iostream>

int main()
{
  std::string splitMe = "aa,b,cc,d";
  std::vector<std::string> tokens;
  boost::algorithm::split(tokens, splitMe, boost::is_any_of(","));

  for (auto token : tokens)
  {
    std::cout << token << std::endl;
  }

  return 0;
}