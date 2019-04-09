#include <iostream>
#include <utap/utap.h>

int main (int argc, char **argv)
{
  UTAP::TimedAutomataSystem tas;

  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " xml-filename" << std::endl;
    return -1;
  }

  std::cout << "loading file " << argv[1] << std::endl;
  try {
    std::cout << parseXMLFile(argv[1], &tas, true) << std::endl;
  } catch(std::exception &e) {
      std::cerr << e.what () << std::endl;
  }

  return 0;
}