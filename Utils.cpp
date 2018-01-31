#include "Utils.h"

namespace sse
{
    // Generate a random key
    string getMasterKey(int keyLength)
    {
        SecByteBlock keyParam(keyLength);
        OS_GenerateRandomBlock(true, keyParam, keyParam.size());
        string key2, key;
        HexEncoder hex(new StringSink(key2));
        hex.Put(keyParam, keyParam.size());
        hex.MessageEnd();
        key.reserve(keyLength);
        for(int i=0; i< 2*keyLength; i+=2)
        {
            string byte = key2.substr(i,2);
            char chr = (char) (int)strtol(byte.c_str(), NULL, 16);
            key.push_back(chr);
        }

        return key;
    }
    
    // Generate keys for file IDs
    string getFjKey(string key, string Fj)
    {
        SHA1 sha1;
        string source = key + Fj;
        string hash;
        StringSource(source, true, new HashFilter(sha1, new StringSink(hash)));
        hash.resize(KEY_LENGTH);
        return hash;
    }
    
    string getFjAddress(string key, int fileNo)
    {
        SHA1 sha1;
        string source = key + to_string(fileNo);
        string hash;
        StringSource(source, true, new HashFilter(sha1, new StringSink(hash)));
        hash.resize(KEY_LENGTH);
        return hash;
    }

    // Generate keys for keywords
    string getWiKey(string key, string Wi, int searchNo)
    {
        SHA1 sha1;
        string source = key + Wi + to_string(searchNo);
        string hash;
        StringSource(source, true, new HashFilter(sha1, new StringSink(hash)));
        hash.resize(KEY_LENGTH);
        return hash;
    }
    
    string getWiAddress(string key, int fileNo)
    {
        SHA1 sha1;
        string source = key + to_string(fileNo) + "0";
        string hash;
        StringSource(source, true, new HashFilter(sha1, new StringSink(hash)));
        hash.resize(KEY_LENGTH);
        return hash;
    }
    
    string getWiMask(string key, int fileNo)
    {
        SHA1 sha1;
        string source = key + to_string(fileNo) + "1";
        string hash;
        StringSource(source, true, new HashFilter(sha1, new StringSink(hash)));
        hash.resize(KEY_LENGTH);
        return hash;
    }

    void str2Char(string source, char dest[])
    {
        for(int i = 0; i < source.size(); i++)
            dest[i] = source[i];
        dest[source.size()] = '\0';
    }

    void char2Str(char source[], string &dest, int len)
    {
        for(int i = 0; i < len; i++)
            dest.push_back(source[i]);
    }

    void writeSetupInfoClient(int fileNo, int wordNo, int pairNo, long iDataSize, int iType)
    {
        ofstream oFile;
        oFile.open (SETUP_FILE_CLIENT, ios::app);
        oFile << fileNo << "\t" << wordNo << "\t" << pairNo << "\t" << iDataSize << "\t" << iType << endl;
        oFile.close();
    }

    void writeSetupTimeClient(int fileNo, int wordNo, int pairNo, double iTime)
    {
        ofstream oFile;
        oFile.open (SETUP_FILE_CLIENT, ios::app);
        oFile << fileNo << "\t" << wordNo << "\t" << pairNo << "\t" << iTime / 1000000 << "\t" << iTime / pairNo << endl;
        oFile.close();
    }

    void writeAddTimeClient(int wordNo, int allFiles, double iTime)
    {
        ofstream oFile;
        oFile.open (ADD_FILE_CLIENT, ios::app);
        oFile << allFiles << "\t" << wordNo << "\t" << iTime / 1000000 << "\t" << iTime / wordNo << endl;
        oFile.close();
    }
    
    void writeSearchTimeClient(int fileNo, int allFiles, double iTime, SearchType opType)
    {
        ofstream oFile;
        oFile.open (SEARCH_FILE_CLIENT, ios::app);
        string strType = opType == SearchType::regular ? "Regular" : "Parallel";
        oFile << allFiles << "\t" << fileNo << "\t" << iTime / 1000000 << "\t" << iTime / fileNo << "\t" << strType << endl;
        oFile.close();
    }
    
    void writeSearchTimeServer(int fileNo, int pairNo, double iTime, SearchType opType)
    {
        ofstream oFile;
        oFile.open (SEARCH_FILE_SERVER, ios::app);
        string strType = opType == SearchType::regular ? "Regular" : "Parallel";
        oFile << pairNo << "\t" << fileNo << "\t" << iTime / 1000000 << "\t" << iTime / fileNo << "\t" << strType << endl;
        oFile.close();
    }
    
    void writeDelTimeServer(int wordNo, int pairNo, double iTime, SearchType opType)
    {
        ofstream oFile;
        oFile.open (DEL_FILE_SERVER, ios::app);
        string strType = opType == SearchType::regular ? "Regular" : "Parallel";
        oFile << pairNo << "\t" << wordNo << "\t" << iTime / 1000000 << "\t" << iTime / wordNo << "\t" << strType << endl;
        oFile.close();
    }
}//namespace