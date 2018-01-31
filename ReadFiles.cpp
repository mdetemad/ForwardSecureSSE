
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
#include "dirent.h"

#define INDEX_FILE_SERVER "index.txt"
#define INVINDEX_FILE_SERVER "invIndex.txt"
#define TIME(t) double(t)/CLOCKS_PER_SEC

using namespace std;
std::map<string, std::vector<string> > file2words, invIndex;
struct BothAreSpaces
{
    char c;
    BothAreSpaces(char r) : c(r) {}
    bool operator()(char l, char r) const
    {
            return r == c && l == c;
    }
};
    

// Process a line to exclude extra characters.
void ProcessLine(string &line)
{
    line.erase(std::remove(line.begin(), line.end(), '.'), line.end());
    line.erase(std::remove(line.begin(), line.end(), '0'), line.end());
    line.erase(std::remove(line.begin(), line.end(), '1'), line.end());
    line.erase(std::remove(line.begin(), line.end(), '2'), line.end());
    line.erase(std::remove(line.begin(), line.end(), '3'), line.end());
    line.erase(std::remove(line.begin(), line.end(), '4'), line.end());
    line.erase(std::remove(line.begin(), line.end(), '5'), line.end());
    line.erase(std::remove(line.begin(), line.end(), '6'), line.end());
    line.erase(std::remove(line.begin(), line.end(), '7'), line.end());
    line.erase(std::remove(line.begin(), line.end(), '8'), line.end());
    line.erase(std::remove(line.begin(), line.end(), '9'), line.end());
    line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
    line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
    std::replace(line.begin(), line.end(), '+', ' ');
    std::replace(line.begin(), line.end(), '-', ' ');
    std::replace(line.begin(), line.end(), '*', ' ');
    std::replace(line.begin(), line.end(), '@', ' ');   
    std::replace(line.begin(), line.end(), ',', ' ');
    std::replace(line.begin(), line.end(), '=', ' ');
    std::replace(line.begin(), line.end(), '_', ' ');
    std::replace(line.begin(), line.end(), ';', ' ');
    std::replace(line.begin(), line.end(), ')', ' ');
    std::replace(line.begin(), line.end(), '(', ' ');
    std::replace(line.begin(), line.end(), '/', ' ');
    std::replace(line.begin(), line.end(), '\\', ' ');
    std::replace(line.begin(), line.end(), '"', ' ');
    std::replace(line.begin(), line.end(), '<', ' ');
    std::replace(line.begin(), line.end(), '>', ' ');
    std::replace(line.begin(), line.end(), '\'', ' ');
    std::replace(line.begin(), line.end(), ':', ' ');
    std::replace(line.begin(), line.end(), '?', ' ');
    std::replace(line.begin(), line.end(), '!', ' ');
    std::replace(line.begin(), line.end(), '[', ' ');
    std::replace(line.begin(), line.end(), ']', ' ');
    std::replace(line.begin(), line.end(), '#', ' ');
    std::replace(line.begin(), line.end(), '\t', ' ');

    while(line.find("  ") != string::npos)
    {
        string::iterator new_end = std::unique(line.begin(), line.end(), BothAreSpaces(' '));
        line.erase(new_end, line.end());               
    }
}
    
void ReadFile(string path, string fileName)
{
    string fullName = fileName;
    if(path.size() > 0)
        fullName = path + "/" + fileName;
    ifstream file(fullName, std::ifstream::in);
    string line, word; 
    while (getline(file, line))
    {
        ProcessLine(line);
        stringstream ss(line);
        while (getline(ss, word, ' '))
        {
            std::transform(word.begin(), word.end(), word.begin(), ::toupper);
            std::transform(fileName.begin(), fileName.end(), fileName.begin(), ::toupper);
            if(word.size() > 5 && word.size() < 10)
                if(std::find(file2words[fileName].begin(), file2words[fileName].end(), word) == file2words[fileName].end()) 
                    file2words[fileName].push_back(word);
        }
    }
}
    
void ReadFiles(string path)
{
    DIR *directoryHandle = opendir(path.c_str());
    if (directoryHandle != NULL)
    {
        dirent *entry = readdir(directoryHandle);
        while (entry != NULL)
        {
            if (DT_DIR == entry->d_type && entry->d_name[0] != '.')
                ReadFiles(path + "/" + entry->d_name);
            else if (DT_REG == entry->d_type)
            {
                string fileName(entry->d_name);
                //if(fileName.find(".h") != string::npos)
                //if(fileName[0] != '.' && fileName.find(".txt") != string::npos)// || fileName.find(".cpp") != string::npos || fileName.find(".h") != string::npos))
                    ReadFile(path, fileName);
            }

            //go to next entry
            entry = readdir(directoryHandle);
        }

        closedir(directoryHandle);
    }
}

void WriteIndex()
{
    ofstream oFile;
    oFile.open (INDEX_FILE_SERVER, ios::out);
    for(map<string, vector<string> >::const_iterator ptr=file2words.begin(); ptr != file2words.end(); ptr++) 
    {
        oFile << "File: " << ptr->first << endl;
        for(vector<string>::const_iterator eptr = ptr->second.begin(); eptr != ptr->second.end(); eptr++)
            oFile << *eptr << " ";
        oFile << endl;
    }

    oFile.close();        
}

void BuildInvertedIndex()
{
    for(map<string, vector<string> >::const_iterator ptr=file2words.begin(); ptr != file2words.end(); ptr++) 
    {
        for(vector<string>::const_iterator eptr = ptr->second.begin(); eptr != ptr->second.end(); eptr++)
        {
            if(std::find(invIndex[*eptr].begin(), invIndex[*eptr].end(), ptr->first) == invIndex[*eptr].end()) 
                invIndex[*eptr].push_back(ptr->first);
        }
    }
}

void WriteInvertedIndex()
{
    ofstream oFile;
    oFile.open (INVINDEX_FILE_SERVER, ios::out);
    for(map<string, vector<string> >::const_iterator ptr=invIndex.begin(); ptr != invIndex.end(); ptr++) 
    {
        oFile << "Keyword: " << ptr->first << endl;
        for(vector<string>::const_iterator eptr = ptr->second.begin(); eptr != ptr->second.end(); eptr++)
            oFile << *eptr << " ";
        oFile << endl;
    }

    oFile.close();        
}

int main()
{
    string path = "/Users/Mohammad/Enron1";
    //string path = "/Users/Mohammad/Downloads/cryptopp565";
    int sTime = clock();
    ReadFiles(path);
    int eTime = clock();
    cout << "Read time: " << TIME(eTime - sTime) << endl;

    // Writing the index down:
    WriteIndex();

    // Building and Writing the inverted index:
    //WriteIndex();
    //ssWriteIndex();
    return 0;
}