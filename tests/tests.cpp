#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>

#include "ArgumentParser.hpp"
#include "Mapping.hpp"
#include "TestArgs.hpp"
#include "TestFilesParsing.hpp"
#include "TestSnmpClient.hpp"
#include "TestHttpClient.hpp"

int main() {

   std::cout << "Argument parsing tests:" << std::endl;
   TestArgs argsTest;
   bool ok1 = argsTest.Run();
   std::cout << std::endl;

   std::cout << "File parsing tests:" << std::endl;
   TestFilesParsing fileTests;
   bool ok2 = fileTests.Run();
   std::cout << std::endl;

   std::cout << "SNMP client tests:" << std::endl;
   TestSnmpClient snmpTests;
   bool ok3 = snmpTests.Run();

    std::cout << "OTPL/HTTP client tests:" << std::endl;
   TestHttpClient httpTests;
   bool ok4 = httpTests.Run();
   std::cout << std::endl;

   if (!ok1 || !ok2 || !ok3 || !ok4) {
       std::cerr << "Some tests failed." << std::endl;
       return 1;
   }
   std::cerr << "All tests passed." << std::endl;
   return 0;
}
