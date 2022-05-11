// To compile: g++ Brute_force_addr_list.cpp -lcurl -o Brute_force_addr_list
#include <cstdio>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <curl/curl.h>

using namespace std;

std::string address_extracted = "";
std::string bash_script_location = "";
std::string prefix = "";
std::string prefix0x = "0x";
std::vector<std::string> addresses_to_check;
int br = 0;

// Run bash script, get it's output and parse it
std::string getCmdOutput(const std::string& mStr) {
  try {
    std::string cmd_out="";
    std::string address="";

    char line[100 + 1] = "";  /* initialize all to 0 ('\0') */
    FILE *script = popen(mStr.c_str(), "r");
    while (fgets(line, sizeof(line), script)) {
        //std::cout << line << std::endl;
        cmd_out=cmd_out+line;
    }
    pclose(script);

    std::size_t found = cmd_out.find("Address:");
    if (found!=std::string::npos){
        // ETH address is 42 chars long
        address = cmd_out.substr (found + 9, 42);
    }
    //std::cout << "[ Address ]: " + address << std::endl;

    return address;
  } catch (const std::exception& e) { // reference to the base of a polymorphic object
     std::cout << e.what(); // information from length_error printed
     return "";
  }
}

void generate_private_key(std::string const& chars, size_t max_len, std::string const& cur) {
  try {
    if(cur.length() == max_len) {
      return;
    }
    else {
      for(auto c : chars) {
        std::string next = cur + c;
        if(next.length() == max_len){
          br++;
          if(br == 1000000){
            exit( 3 );
          }
          // Private key to Public key to Address
          bash_script_location = "printf "+ prefix + next +" | ./priv_pub_generate_openssl.sh";
          address_extracted = getCmdOutput(bash_script_location);
          // Write to file
          if(std::find(addresses_to_check.begin(), addresses_to_check.end(), address_extracted) != addresses_to_check.end()){
            std::cout << "[MATCH] [PRIVATE KEY]: " + prefix0x + prefix + next + " [ADDRESS]: " + address_extracted << std::endl;
            std::ofstream outfile;
            outfile.open("LOG_brute_list.txt", std::ios_base::app);
            outfile << "[MATCH] [PRIVATE KEY]: " + prefix0x + prefix + next + " [ADDRESS]: " + address_extracted << std::endl;
          } else {
            std::cout << to_string(br) + " [PRIVATE KEY]: " + prefix0x + prefix + next + " [ADDRESS]: " + address_extracted << std::endl;
          }
        }
        generate_private_key(chars, max_len, next);
      }
    }
  } catch (const std::exception& e) { // reference to the base of a polymorphic object
     std::cout << e.what(); // information from length_error printed
  }
}

int main() {
  // Load addresses to check against
  std::string line;    
  ifstream fin("extract_eth_top_balance_addresses.txt");
  while(getline(fin,line)){
      addresses_to_check.push_back(line);
  }

  // Load config values
  std::ifstream infile("config.txt"); // Open input file
  std::string charList = "";
  std::string charLengthTemp = "";

  getline(infile, charList); // Get first line.
  getline(infile, charLengthTemp); // Get second line.
  getline(infile, prefix); // Get third line.

  std::cout << "Char list: " + charList << std::endl;
  std::cout << "Char Length: " + charLengthTemp << std::endl;

  int charLength = std::stoi (charLengthTemp);

  // Start generating addresses
  generate_private_key(charList, charLength, "");
  return 0;
}