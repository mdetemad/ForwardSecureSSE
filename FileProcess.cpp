#include "Utils.h"
#include "dirent.h"
#include "FileProcess.h"

namespace sse
{
    FileReading::FileReading()
    {
    }

    void FileReading::ReadFile(string path, string fileName)
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
    
    // Process a line to exclude extra characters.
    void FileReading::ProcessLine(string &line)
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
        line.erase(std::remove(line.begin(), line.end(), '['), line.end());
        line.erase(std::remove(line.begin(), line.end(), ']'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '{'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '}'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '|'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
        std::replace(line.begin(), line.end(), '+', ' ');
        std::replace(line.begin(), line.end(), '-', ' ');
        std::replace(line.begin(), line.end(), '_', ' ');
        std::replace(line.begin(), line.end(), '*', ' ');
        std::replace(line.begin(), line.end(), '@', ' ');   
        std::replace(line.begin(), line.end(), ',', ' ');
        std::replace(line.begin(), line.end(), '=', ' ');
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
        std::replace(line.begin(), line.end(), '&', ' ');
        
        while(line.find("  ") != string::npos)
        {
            string::iterator new_end = std::unique(line.begin(), line.end(), BothAreSpaces(' '));
            line.erase(new_end, line.end());               
        }
    }
    
    void FileReading::ReadFiles(string path)
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
                    ReadFile(path, fileName);
                }

                //go to next entry
                entry = readdir(directoryHandle);
            }
            
            closedir(directoryHandle);
        }
    }
    
    bool FileReading::IsEnglish(string word)
    {
        for(int i = 0; i < word.size(); i++)
            if(!isalpha(word[i]))
                return false;
        return true;
    }
    
    int FileReading::ReadWikiArchives(string &error)
    {
        int fileID = 100;
        string word = "", fileName = "", fileText = "", strPath;
        DIR *directoryHandle = opendir("/home/ubuntu/WikiData");
        if (directoryHandle != NULL)
        {
            dirent *entry = readdir(directoryHandle);
            while (entry != NULL)
            {
                if (entry->d_type = DT_REG && entry->d_name[0] != '.')
                {
                    strPath = "/home/ubuntu/WikiData/";
                    cout << "Reading " << entry->d_name << endl;
                    strPath += entry->d_name;
                    pugi::xml_document doc;
                    xml_parse_result result = doc.load_file(strPath.c_str());
                    if(!result)
                    {
                        error = "XML file '";
                        error += strPath.c_str();
                        error += "' parsed with error: ";
                        error += result.description();
                        error += "\tStatus: ";
                        error += result.status;
                        error += "\tOffset: ";
                        error += result.offset;
                        cout << error << endl;
                        //return -1;
                    }else
                    {
                        xml_node rootNode = doc.child("mediawiki");
                        for(xml_node page : rootNode.children("page"))
                        {
                            fileText = page.child("revision").child("text").child_value();
                            if(fileText.length() < 200)
                                continue;
                            fileName = to_string(fileID++);
                            if (fileID % 10000 == 0)
                                cout << "Processing page No: " << fileID << endl;
                            ProcessLine(fileText);
                            stringstream ss(fileText);
                            while (getline(ss, word, ' '))
                            {
                                if(!IsEnglish(word))
                                    continue;
                                std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                                Porter2Stemmer::stem(word);
                                if(word.size() >= 5 && word.size() <= 10)
                                {
                                    std::transform(word.begin(), word.end(), word.begin(), ::toupper);
                                    if(std::find(file2words[fileName].begin(), file2words[fileName].end(), word) == file2words[fileName].end()) 
                                        file2words[fileName].push_back(word);
                                }
                            }
                        }
                    }
                }

                //go to next entry
                entry = readdir(directoryHandle);
            }
            
            closedir(directoryHandle);
        }

        return 0;
    }

    int FileReading::ReadWikiArchive(string &error)
    {
        pugi::xml_document doc;
        xml_parse_result result = doc.load_file("xaa.xml");
        if(!result)
        {
            error = "XML file 'xaa.xml' parsed with error: ";
            error += result.description();
            error += "\tStatus: ";
            error += result.status;
            error += "\tOffset: ";
            error += result.offset;
            return -1;
        }

        int fileID = 100;
        string word = "", fileName = "", fileText = "";
        xml_node rootNode = doc.child("mediawiki");
        //xml_node page = rootNode.child("page");
        //xml_node paget = page.child("text");
        //cout << "page text: " << paget.name() << endl;
        for(xml_node page : rootNode.children("page"))
        {
            fileName = to_string(fileID++);
            fileText = page.child("revision").child("text").child_value();
            //cout << "Page title: " << page.child("revision").child("text").child_value() << endl << endl;
            if(fileText.length() < 100)
                continue;
            ProcessLine(fileText);
            stringstream ss(fileText);
            while (getline(ss, word, ' '))
            {
                if(!IsEnglish(word))
                    continue;
                std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                Porter2Stemmer::stem(word);
                if(word.size() >= 5 && word.size() <= 10)
                {
                    std::transform(word.begin(), word.end(), word.begin(), ::toupper);
                    if(std::find(file2words[fileName].begin(), file2words[fileName].end(), word) == file2words[fileName].end()) 
                        file2words[fileName].push_back(word);
                }
            }
        }

        return 0;
    }

    void FileReading::AddKeyword(string masterKey, string FjKey, string keywords, string fileName, SSEMap &TW, SSEMap &TF, MWMap &MW, MFMap &MF, SSEType sseType)
    {
        string word = "";
        stringstream ss(keywords);
        int noFiles = 0, noWords = 0;
        while (getline(ss, word, ' '))
        {
            string FjAddress = "";
            mtx.lock();
            MW[word].noFiles++;
            MF[fileName]++;
            noFiles = MW[word].noFiles;
            noWords = MF[fileName];
            mtx.unlock();

            if(sseType == SSEType::forward)
                FjAddress = getFjAddress(FjKey, noWords);
            string WiKey = getWiKey(masterKey, word, 0);
            string WiAddress = getWiAddress(WiKey, noFiles);
            string WiMask = getWiMask(WiKey, noFiles);

            // Mask the file name.
            string WiValMasked = fileName;
            for(int i = 0; i < WiValMasked.size(); i++)
                WiValMasked[i] = WiValMasked[i]^WiMask[i];
            string WiVal = FjAddress + WiValMasked;

            // Add the values into maps
            mtx.lock();
            if(sseType == SSEType::forward)
                TF[FjAddress] = WiAddress;
            TW[WiAddress] = WiVal;
            mtx.unlock();
        }
    }

    void FileReading::ReadIndex(string masterKey, SSEMap &TW, SSEMap &TF, MWMap &MW, MFMap &MF, SSEType sseType)
    {
        int processed = 0;
        bool fileOK = false;
        string line, word, fileName, FjKey, value;
        ifstream indexFile(INDEX_FILE_SERVER, std::ifstream::in);
        while (getline(indexFile, line))
        {
            stringstream ss(line);
            while (getline(ss, word, ' '))
            {
                if(word == "File:")
                {
                    getline(ss, word, ' ');
                    fileName = word;
                    // If the file does not exists already.
                    fileOK = (MF.find(fileName) == MF.end());
                    // Generate the file key.
                    if(fileOK)
                        FjKey = getFjKey(masterKey, fileName);
                    //cout << "FileName: " << fileName << endl;
                    if(processed == 10000)
                        return;
                    processed++;
                    if(processed % 5000 == 0)
                        cout << "processed files: " << processed << endl;
                }else
                {
                    if(fileOK)
                    {
                        string FjAddress = "";
                        if(sseType == SSEType::forward)
                            FjAddress = getFjAddress(FjKey, MF[fileName]);
                        string WiKey = getWiKey(masterKey, word, MW[word].noSearch);
                        string WiAddress = getWiAddress(WiKey, MW[word].noFiles);
                        string WiMask = getWiMask(WiKey, MW[word].noFiles);

                        // Mask the file name.
                        string WiValMasked = fileName;
                        for(int i = 0; i < WiValMasked.size(); i++)
                            WiValMasked[i] = WiValMasked[i]^WiMask[i];
                        string WiVal = FjAddress + WiValMasked;
                        
                        // Add the values into maps
                        if(sseType == SSEType::forward)
                            TF[FjAddress] = WiAddress;
                        TW[WiAddress] = WiVal;
                        MW[word].noFiles++;
                        MF[fileName]++;
                    }
                }
            }
        }
    }
    
    void FileReading::ConvertIndex2Sophos()
    {
        int processed = 0;
        bool fileOK = false, bFirst = true;
        string line, word, fileName, FjKey;
        ifstream inFile("index.txt", std::ifstream::in);
        while (getline(inFile, line))
        {
            stringstream ss(line);
            while (getline(ss, word, ' '))
            {
                if(word == "File:")
                {
                    getline(ss, fileName, ' ');
                    processed++;
                }else
                {
                    std::transform(word.begin(), word.end(), word.begin(), ::toupper);
                    //if(std::find(file2words[word].begin(), file2words[word].end(), fileName) == file2words[word].end()) 
                    file2words[word].push_back(fileName);                    
                }
            }

            if(processed % 10000 == 0)
                cout << "processed files: " << processed << endl;
            if(processed == 10000 || processed == 20000 || processed == 50000 || processed == 100000 || processed == 250000 || processed == 500000 || processed == 750000 || processed == 1000000 || processed == 1500000 || processed == 2000000 || processed == 2500000 || processed == 3000000)
            {
                if(bFirst)
                    WriteIndexSophos(processed);
                bFirst = !bFirst;   
            }
        }

        WriteIndexSophos(0);
        inFile.close();
    }

    void FileReading::WriteIndexSophos(int nProcessed)
    {
        string fileName("indexSophos");
        fileName += to_string(nProcessed);
        ofstream oFile;
        oFile.open (fileName, ios::out);
        oFile << "{" << endl;
        for(map<string, vector<string> >::const_iterator ptr=file2words.begin(); ptr != file2words.end(); ptr++) 
        {
            oFile << "\"" << ptr->first << "\":[" ;
            vector<string>::const_iterator eptr = ptr->second.begin();
            oFile << *eptr;
            eptr++;
            for(; eptr != ptr->second.end(); eptr++)
                oFile << ", " << *eptr;
            oFile << "]," << endl;
        }

        oFile << "}" << endl;
        oFile.close();        
    }

    void FileReading::BuildIndex(string masterKey, SSEMap &TW, SSEMap &TF, MWMap &MW, MFMap &MF, SSEType sseType)
    {
        int processed = 0;
        for(map<string, vector<string> >::const_iterator ptr=file2words.begin(); ptr != file2words.end(); ptr++) 
        {
            processed++;
            if(processed % 1000 == 0)
                cout << "processed files: " << processed << endl;
            // If the file does not exists already.
            if(MF.find(ptr->first) == MF.end())
            {
                //cout << "File: " << ptr->first << endl;
                // Generate the file key.
                string FjKey = getFjKey(masterKey, ptr->first);
                MF[ptr->first] = ptr->second.size();
                int wordNo = 0;
                for(vector<string>::const_iterator eptr = ptr->second.begin(); eptr != ptr->second.end(); eptr++)
                {
                    string FjAddress = getFjAddress(FjKey, wordNo);
                    /*
                    if(MW.find(*eptr) == MW.end())
                    {
                        MW[*eptr].noFiles = 0;    //  No of files containing this word
                        MW[*eptr].noSearch = 0;   //  No of searches
                    }
                    */
                    string WiKey = getWiKey(masterKey, *eptr, MW[*eptr].noSearch);
                    string WiAddress = getWiAddress(WiKey, MW[*eptr].noFiles);
                    string WiMask = getWiMask(WiKey, MW[*eptr].noFiles);

                    // Mask the file name.
                    string WiValMasked = ptr->first;
                    for(int i = 0; i < WiValMasked.size(); i++)
                        WiValMasked[i] = WiValMasked[i]^WiMask[i];
                    string WiVal = FjAddress + WiValMasked;
                    
                    // Add the values into maps
                    
                    if(TF.find(FjAddress) == TF.end())
                        TF[FjAddress] = WiAddress;
                    else
                        cout << "Duplicate" << endl;
                    if(TW.find(WiAddress) == TW.end())
                        TW[WiAddress] = WiVal;
                    else
                        cout << "Duplicate: " << ptr->first << " " << *eptr << endl;
                    
                    MW[*eptr].noFiles++;
                    wordNo++;
                }
                
                MF[ptr->first] = wordNo;
            }
        }
        
        file2words.clear();
    }
    
    void FileReading::WriteIndex()
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

    void FileReading::BuildInvertedIndex()
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

    void FileReading::WriteInvertedIndex()
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

}//namespace
