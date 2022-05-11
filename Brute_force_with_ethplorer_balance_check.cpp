// To compile: g++ Brute_force_with_ethplorer_balance_check.cpp -lcurl -o Brute_force_with_ethplorer_balance_check
#include <cstdio>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <curl/curl.h>

using namespace std;

std::string address_extracted = "";
std::string bash_script_location = "";
int balance = 0;
std::string prefix = "";
std::string prefix0x = "0x";
int api_key_num = 0;
const char *api_key_list[2] = { "?apiKey=EK-4Ek5d-V89bYqU-obyyN", "?apiKey=EK-4Ek5d-V89bYqU-obyyN" };
//const char *api_key_list[2] = { "?apiKey=EK-4FCtv-vavij57-3GQYQ", "?apiKey=EK-4Ek5d-V89bYqU-obyyN" }; // some activation bs
std::string api_url = "https://api.ethplorer.io/getAddressInfo/";
//std::string api_url = "https://api.ethplorer.io/getAddressInfo/0x004462cd0e4c752bc5211163c6c6b5e184be8ba3?apiKey=EK-q8iS1-zVRyCU1-fowLw";

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
  
int extract_int_from_string(string str) {
  try {
    stringstream str_strm;
    str_strm << str; //convert the string s into stringstream
    string temp_str;
    int temp_int;
    while(!str_strm.eof()) {
        str_strm >> temp_str; //take words into temp_str one by one
        if(stringstream(temp_str) >> temp_int) { //try to convert string to int
          //cout << temp_int << " ";
        }
        temp_str = ""; //clear temp string
    }
    return temp_int;
  } catch (const std::exception& e) { // reference to the base of a polymorphic object
     std::cout << e.what(); // information from length_error printed
     return 0;
  }
} 

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  try {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
  } catch (const std::exception& e) { // reference to the base of a polymorphic object
     std::cout << e.what(); // information from length_error printed
     return 0;
  }
}

int curl_call(std::string publicAddr, std::string privKey) {
  try {
    if(api_key_num == sizeof api_key_list / sizeof api_key_list[0] - 1)
      api_key_num = 0;
    else
      api_key_num++;
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    std::string url_final = api_url + publicAddr + api_key_list[api_key_num];
    std::string balance_tmp = "0";
    int balance_final = 0;

    curl = curl_easy_init();
    if(curl) {
      curl_easy_setopt(curl, CURLOPT_URL, url_final.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
      res = curl_easy_perform(curl);
      curl_easy_cleanup(curl);

      //std::cout << readBuffer << std::endl;
      std::ofstream outfile;
      std::size_t found = readBuffer.find(":{\"balance\":");
      if (found!=std::string::npos) {
        balance_tmp = readBuffer.substr(found + 12, 4);
        balance_final = extract_int_from_string(balance_tmp);
        outfile.open("LOG_curl_balance_response.txt", std::ios_base::app);
        outfile << "[PRIVATE_KEY]: " + privKey + " [ADDR]: " + publicAddr + " [BALANCE]: " + to_string(balance_final) << std::endl;
        //outfile << readBuffer << std::endl;
        return balance_final;
      } else {
        std::cout << "API ERROR: " << api_key_list[api_key_num] << std::endl;
        outfile.open("LOG_curl_balance_response.txt", std::ios_base::app);
        outfile << "[PRIVATE_KEY]: " + privKey + " [ADDR]: " + publicAddr + " [API ERROR]: " + api_key_list[api_key_num] << std::endl;
        //outfile << readBuffer << std::endl;
        return -1;
      }
    } else {
      return -1;
    }
  } catch (const std::exception& e) { // reference to the base of a polymorphic object
     std::cout << e.what(); // information from length_error printed
     return -1;
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
          // Private key to Public key to Address
          bash_script_location = "printf "+ prefix + next +" | ./priv_pub_generate_openssl.sh";
          address_extracted = getCmdOutput(bash_script_location);
          balance = curl_call(address_extracted, prefix0x + prefix + next);
          // Write to file
          if(balance >= 0) {
            std::cout << "[PRIVATE KEY]: " + prefix0x + prefix + next + " [ADDRESS]: " + address_extracted + " [BALANCE]: " + to_string(balance) << std::endl;
            std::ofstream outfile;
            outfile.open("LOG_brute_list.txt", std::ios_base::app);
            outfile << "[PRIVATE KEY]: " + prefix0x + prefix + next + " [ADDRESS]: " + address_extracted + " [BALANCE]: " + to_string(balance) << std::endl;
          } else if(balance == -1) {
            // API ERROR OR EXCEPTION
            std::cout << "[PRIVATE KEY]: " + prefix0x + prefix + next + " [ADDRESS]: " + address_extracted + " [API ERROR]: " + to_string(balance) << std::endl;
            std::ofstream outfile;
            outfile.open("LOG_brute_list.txt", std::ios_base::app);
            outfile << "[PRIVATE KEY]: " + prefix0x + prefix + next + " [ADDRESS]: " + address_extracted + " [API ERROR]: " + to_string(balance) << std::endl;
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
  std::ifstream infile("config.txt"); // Open input file
  std::string charList = "";
  std::string charLengthTemp = "";

  getline(infile, charList); // Get first line.
  getline(infile, charLengthTemp); // Get second line.
  getline(infile, prefix); // Get third line.

  std::cout << "Char list: " + charList << std::endl;
  std::cout << "Char Length: " + charLengthTemp << std::endl;

  int charLength = std::stoi (charLengthTemp);

  generate_private_key(charList, charLength, "");
  return 0;
}