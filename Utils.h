#pragma once

#include <map>
#include <vector>
#include <ctime>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <mutex>
#include <thread>
#include <future>
#include <chrono>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include "cryptopp/sha.h"
#include "cryptopp/filters.h"
#include "cryptopp/hex.h"
#include <cryptopp/osrng.h>
#include <sparsehash/sparse_hash_map>
#include <utility>
#include <unordered_map>

#define KEY_LENGTH 16
#define BUFF_LENGTH 8192
#define MAX_LINE 100
#define PARALLEL_NO 5
#define GAP_STRING "ABCDE"
#define DEL_STRING "DELETED"
#define END_STRING "ENDENDEND"
#define TIME(t) double(t)/CLOCKS_PER_SEC
#define CHRONO(t1, t2) std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()

#define SEARCH_FILE_INPUT "searchWords.txt"
#define PSEARCH_FILE_INPUT "parSearchWords.txt"
#define DELETE_FILE_INPUT "delFiles.txt"

#define SETUP_FILE_CLIENT "setupTimesClient.txt"
#define ADD_FILE_CLIENT "addTimesClient.txt"
#define ADD_FILE_SERVER "addTimesServer.txt"
#define SEARCH_FILE_CLIENT "searchTimesRoundTrip.txt"
#define SEARCH_FILE_SERVER "searchTimesServer.txt"
#define DEL_FILE_SERVER "delTimesServer.txt"
#define INDEX_FILE_SERVER "index.txt"
#define INVINDEX_FILE_SERVER "invIndex.txt"

using namespace std; 
using namespace CryptoPP; 

namespace sse
{
    enum SSEType {forward, backward};
    enum SearchType {regular, parallel};
    struct StructMW
    {
        int noFiles;
        int noSearch;
    };
    
    //typedef std::map<string, string> SSEMap;
    typedef google::sparse_hash_map<string, string> SSEMap;
    typedef std::map<string, int> MFMap;
    typedef std::map<string, StructMW> MWMap;
    
    string getMasterKey(int keyLength);
    string getFjKey(string key, string Fj);
    string getWiKey(string key, string Wi, int searchNo);
    string getWiAddress(string key, int fileNo);
    string getWiMask(string key, int fileNo);
    string getFjAddress(string key, int fileNo);

    void str2Char(string source, char dest[]);
    void char2Str(char source[], string &dest, int len);

    void writeSetupInfoClient(int fileNo, int wordNo, int pairNo, long iDataSize, int iType);
    void writeSetupTimeClient(int fileNo, int wordNo, int pairNo, double iTime);
    void writeAddTimeClient(int wordNo, int allFiles, double iTime);
    void writeSearchTimeClient(int fileNo, int allFiles, double iTime, SearchType opType);
    void writeSearchTimeServer(int fileNo, int pairNo, double iTime, SearchType opType);
    void writeDelTimeServer(int wordNo, int pairNo, double iTime, SearchType opType);
}//namespace