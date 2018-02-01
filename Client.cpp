/*
 Forward- and Backeard-private Searchable Symmetric Encryption
 Copyright (C) 2016 Mohammad Etemad

 This is a free software: you can redistribute/modify it
 under the GNU Affero General Public License.

 This is distributed freely hoping to help further improvements in the
 searchable encryption area, as is and WITHOUT ANY WARRANTY.
 See the GNU Affero General Public License for more details.
*/


#include "Client.h"

namespace sse
{
	Client::Client(int serverPort, const string serverAddr): connIndex(serverPort, serverAddr)
	{
	    masterKey = getMasterKey(AES::DEFAULT_KEYLENGTH);
        string strNull("NULL");
    }

    int Client::Connect(string &error)
	{
	    return connIndex.Connect(error);
	}

	void Client::Close()
	{
		connIndex.Close();
	}

    void Client::ComputeMax()
    {
        int processed = 0, fileNo = 1000000;
        bool fileOK = false, assigned = false;
        string line, fileName, FjKey, value, word;
        ifstream indexFile(INDEX_FILE_SERVER, std::ifstream::in);
        while (getline(indexFile, line))
        {
            if(line.substr(0, 5) == "File:")
            {
                fileName = to_string(fileNo++);
                if(processed == 1000000)
                    break;
                processed++;
                if(processed % 5000 == 0)
                    cout << "Processed files: " << processed << endl;
            }else
            {
                stringstream ss(line);
                while (getline(ss, word, ' '))
                {
                    MW[word].noFiles++;
                    MF[fileName]++;
                }
            }
        }

        int nMax = 0;
        string maxWord = "";
        for(MWMap::const_iterator ptr = MW.begin(); ptr != MW.end(); ptr++)
        {
            if(ptr->second.noFiles > 250000)
                cout << ptr->first << ": " << ptr->second.noFiles << endl;
            if(ptr->second.noFiles > nMax)
            {
                nMax = ptr->second.noFiles;
                maxWord = ptr->first;
            }
        }

        cout << maxWord << ": " << nMax << endl;
        nMax = 0;
        for(MFMap::const_iterator ptr = MF.begin(); ptr != MF.end(); ptr++)
        {
            if(ptr->second > 5000)
                cout << ptr->first << ": " << ptr->second << endl;
            if(ptr->second > nMax)
            {
                nMax = ptr->second;
                maxWord = ptr->first;
            }
        }

        cout << maxWord << ": " << nMax << endl;
    }

    int Client::AddKeyword(string FjKey, string fileName, vector<wordBox> wb, int &totalSent)
    {
        //cout << fileName << "\t" << wb.size() << endl;
        string data2SendTF("BUILD"), data2SendTW("BUILD");
        data2SendTW += GAP_STRING;
        data2SendTW += "TW";
        data2SendTW += GAP_STRING;
        data2SendTF += GAP_STRING;
        data2SendTF += "TF";
        data2SendTF += GAP_STRING;
        for (std::vector<wordBox>::iterator it = wb.begin(); it != wb.end(); ++it)
        {
            //cout << it->word << "\t" << it->noFiles << endl;
            string FjAddress = getFjAddress(FjKey, it->noWords);
            string WiKey = getWiKey(masterKey, it->word, 0);
            string WiAddress = getWiAddress(WiKey, it->noFiles);
            string WiMask = getWiMask(WiKey, it->noFiles);

            // Mask the file name.
            string WiValMasked = fileName;
            for(int i = 0; i < WiValMasked.size(); i++)
                WiValMasked[i] = WiValMasked[i]^WiMask[i];
            string WiVal = FjAddress + WiValMasked;

            if(data2SendTF.size() + FjAddress.size() + WiAddress.size() + sizeof(GAP_STRING) > BUFF_LENGTH)
            {
                mtx.lock();
                if (connIndex.SendData(data2SendTF, error) == -1)
                {
                    error = "Error in uploading TF data...";
                    return -1;
                }

                mtx.unlock();
                data2SendTF = "TF";
                data2SendTF += GAP_STRING;
            }

            data2SendTF += (FjAddress + WiAddress + GAP_STRING);
            totalSent += (FjAddress.size() + WiAddress.size());
            if(data2SendTW.size() + WiAddress.size() + WiVal.size() + sizeof(GAP_STRING) > BUFF_LENGTH)
            {
                mtx.lock();
                if (connIndex.SendData(data2SendTW, error) == -1)
                {
                    error = "Error in uploading TF data...";
                    return -1;
                }

                mtx.unlock();
                data2SendTW = "TW";
                data2SendTW += GAP_STRING;
            }

            data2SendTW += (WiAddress + WiVal + GAP_STRING);
            totalSent += (WiAddress.size() + WiVal.size());
        }

        if(data2SendTF.size() > (sizeof(GAP_STRING) + 2))
        {
            mtx.lock();
            if (connIndex.SendData(data2SendTF, error) == -1)
            {
                error = "Error in uploading TF data...";
                return -1;
            }
            mtx.unlock();
        }

        if(data2SendTW.size() > (sizeof(GAP_STRING) + 2))
        {
            mtx.lock();
            if (connIndex.SendData(data2SendTW, error) == -1)
            {
                error = "Error in uploading TW data...";
                return -1;
            }
            mtx.unlock();
        }

        return 0;
    }

    void Client::ReadFromIndex(int &nPairs)
    {
        int processed = 0, totalSent = 0, fileNo = 1000000;
        bool fileOK = false, assigned = false;
        string line, fileName, FjKey, value, word;
        ifstream indexFile(INDEX_FILE_SERVER, std::ifstream::in);
        vector<future<int>> futures;
        while (getline(indexFile, line))
        {
            if(line.substr(0, 5) == "File:")
            {
                fileName = to_string(fileNo++);
                // If the file does not exists already.
                fileOK = (MF.find(fileName) == MF.end());
                // Generate the file key.
                if(fileOK)
                    FjKey = getFjKey(masterKey, fileName);
                if(processed == 5000)
                    return;
                processed++;
                if(processed % 10000 == 0)
                    cout << "processed files: " << processed << endl;
            }else
            {
                vector<wordBox> wordBoxes;
                stringstream ss(line);
                while (getline(ss, word, ' '))
                {
                    wordBox wb;
                    wb.word = word;
                    wb.noFiles = MW[word].noFiles++;
                    wb.noWords = MF[fileName]++;
                    wordBoxes.push_back(wb);
                }

                nPairs += wordBoxes.size();
                assigned = false;
                while(!assigned)
                {
                    if(futures.size() < PARALLEL_NO)
                    {
                        futures.push_back(async(launch::async, &Client::AddKeyword, this, FjKey, fileName, wordBoxes, std::ref(totalSent)));
                        assigned = true;
                    }else
                    {
                        for(int i = 0; i < PARALLEL_NO; ++i)
                        {
                            if(futures[i].wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
                            {
                                futures.erase(futures.begin() + i);
                                futures.push_back(async(launch::async, &Client::AddKeyword, this, FjKey, fileName, wordBoxes, std::ref(totalSent)));
                                assigned = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    int Client::PreProcess(string &error)
	{
        int nPairs = 0;
        auto sTime = chrono::system_clock::now();
        ReadFromIndex(nPairs);
        auto eTime = chrono::system_clock::now();
		writeSetupTimeClient(MF.size(), MW.size(), nPairs, CHRONO(sTime, eTime));

        int dataSize = 0;
        for(MWMap::const_iterator ptr = MW.begin(); ptr != MW.end(); ptr++)
        {
			//dataSize += ptr->first.size() + sizeof(ptr->second);
            if(ptr->second.noFiles > 10000)
                cout << ptr->first << ": " << ptr->second.noFiles << endl;
        }

		for(MFMap::const_iterator ptr = MF.begin(); ptr != MF.end(); ptr++)
        {
			//dataSize += ptr->first.size() + sizeof(ptr->second);
            if(ptr->second > 4000)
                cout << ptr->first << endl;
        }

        //writeSetupInfoClient(MF.size(), MW.size(), TF.size(), dataSize, 0);
        return 0;
	}

	int Client::Upload(string &error)
	{
		// Tell the server the setup is in progress...
        long totalSent = 0;
		string data2Send("BUILD");
		data2Send += GAP_STRING;

        // Start with TF
        data2Send += "TF";
        data2Send += GAP_STRING;
        for(SSEMap::const_iterator ptr = TF.begin(); ptr != TF.end(); ptr++)
        {
            if(data2Send.size() + ptr->first.size() + ptr->second.size() + sizeof(GAP_STRING) > BUFF_LENGTH)
            {
                if (connIndex.SendData(data2Send, error) == -1)
                {
                    error = "Error in uploading TF data...";
                    return -1;
                }

                data2Send = "";
            }

            data2Send += (ptr->first + ptr->second + GAP_STRING);
            totalSent += ptr->first.size() + ptr->second.size();
        }

        if(data2Send.size() > 0)
        {
            if (connIndex.SendData(data2Send, error) == -1)
            {
                error = "Error in uploading TF data...";
                return -1;
            }
        }

		// Then send TW
		data2Send += "TW";
		data2Send += GAP_STRING;
		for(SSEMap::const_iterator ptr = TW.begin(); ptr != TW.end(); ptr++)
		{
			if(data2Send.size() + ptr->first.size() + ptr->second.size() + sizeof(GAP_STRING) > BUFF_LENGTH)
			{
				if (connIndex.SendData(data2Send, error) == -1)
				{
					error = "Error in uploading TW data...";
					return -1;
				}

				data2Send = "";
			}

			data2Send += (ptr->first + ptr->second + GAP_STRING);
			totalSent += ptr->first.size() + ptr->second.size();
		}

		if(data2Send.size() > 0)
		{
			if (connIndex.SendData(data2Send, error) == -1)
			{
				error = "Error in uploading TW data...";
				return -1;
			}
		}

		TW.clear();
		TF.clear();
		return 0;
	}

	int Client::Search(string keyword, int &fileNo, SearchType opType, string &error)
	{
        std::transform(keyword.begin(), keyword.end(), keyword.begin(), ::tolower);
        Porter2Stemmer::stem(keyword);
        std::transform(keyword.begin(), keyword.end(), keyword.begin(), ::toupper);
		if(MW.find(keyword) == MW.end())
		{
			cout << "The keyword '" << keyword << "' does not exists in the dictionary." << endl;
			return 0;
		}

        // Tell the server I am going to start a search.
		string data2Send;
        data2Send = opType == SearchType::regular ? "SEARCH" : "PSEARCH";
		data2Send += GAP_STRING;
		string WiOldKey = getWiKey(masterKey, keyword, MW[keyword].noSearch);
		data2Send += WiOldKey;
		data2Send += to_string(MW[keyword].noFiles);
		data2Send += GAP_STRING;
        auto sTime = chrono::system_clock::now();
		if (connIndex.SendData(data2Send, error) == -1)
		{
			error = "Search request is not sent...";
			return -1;
		}

		// Increase searchNo: Wi is searched for once more. Then generate the new key for Wi.
		MW[keyword].noSearch++;
		string mask, WiNewKey = getWiKey(masterKey, keyword, MW[keyword].noSearch);
		int nReceived = 0, nCur = 0, nRead = 0, nFile = 0;
        int keyLength = KEY_LENGTH;
		char readTill[3*KEY_LENGTH], fName[KEY_LENGTH];
		bool bEnd = false;
        fileNo = 0;
        vector<string> recPairs;
		while (!bEnd && (nReceived = recv(connIndex.socketDescriptor, buffer, BUFF_LENGTH, 0)) > 0)
		{
			nCur = 0;
			cout << "nReceived: " << nReceived << endl;
			//cout << "buffer: " << buffer << endl;
			while(nCur < nReceived)
			{
				readTill[nRead++] = buffer[nCur++];
				// If end of segment of data reached
				if(nRead > 4 && readTill[nRead - 1] == 'E' && readTill[nRead - 2] == 'D' && readTill[nRead - 3] == 'C' && readTill[nRead - 4] == 'B' && readTill[nRead - 5] == 'A')
				{
					nRead = nRead - 5;
					readTill[nRead] = '\0';
                    //cout << "nRead: " << nRead << "readTill: " << readTill << endl;
					if(!strcmp(readTill, END_STRING))
						bEnd = true;
					else if(strcmp(readTill, DEL_STRING))
                    {
                        //mask = getWiMask(WiOldKey, nFile++);
                        //for(int i = 0; i < nRead; i++)
                            //readTill[i] = readTill[i]^mask[i];
                        string strData = "";
                        char2Str(readTill, strData, nRead);
                        recPairs.push_back(strData);
                    }

                    nRead = 0;
                }
            }
        }

        cout << endl << endl;
        auto eTime = chrono::system_clock::now();
		writeSearchTimeClient(recPairs.size(), MF.size(), CHRONO(sTime, eTime), opType);
        for (vector<string>::iterator it = recPairs.begin(); it != recPairs.end(); ++it)
        {
            str2Char(*it, readTill);
            nRead = (*it).size();
            int cnt = 0;
            for(cnt = 0; cnt < nRead - keyLength; cnt++)
                fName[cnt] = readTill[cnt + keyLength];
            fName[cnt] = '\0';
            cout << fName << "\n";
            // If the file is deleted already
            if(MF.find(fName) == MF.end())
                continue;

            // Now, prepare the received data with the new key.
            string WiNewAddress = getWiAddress(WiNewKey, fileNo);
            string WiNewMask = getWiMask(WiNewKey, fileNo);
            for(int i = keyLength; i < nRead; i++)
                readTill[i] = fName[i - keyLength]^WiNewMask[i - keyLength];
            string WiVal = "", FjAddress = "";
            char2Str(readTill, WiVal, nRead);
            TW[WiNewAddress] = WiVal;
            nRead = 0;
            fileNo++;
            char2Str(readTill, FjAddress, KEY_LENGTH);
            TF[FjAddress] = WiNewAddress;
        }

		MW[keyword].noFiles = fileNo;
        cout << endl << fileNo << "\tFiles received." << endl << endl;
		return 0;
	}

	int Client::SearchBatch(string &error)
	{
		string line;
        int fileNo = 0;
		ifstream file(SEARCH_FILE_INPUT, std::ifstream::in);
		while (getline(file, line))
		{
            cout << "Searching for : " << line << endl;
			for(int i = 0; i < 1; i++)
            {
				if(Search(line, fileNo, SearchType::regular, error) == -1)
					return -1;
                if(Upload(error) == -1)
                    return -1;
            }
		}

		return 0;
	}

	int Client::ParallelSearchBatch(string &error)
	{
		string line;
        int fileNo = 0;
		ifstream file(SEARCH_FILE_INPUT, std::ifstream::in);
		while (getline(file, line))
		{
            cout << "Searching for : " << line << endl;
			for(int i = 0; i < 1; i++)
            {
				if(Search(line, fileNo, SearchType::parallel, error) == -1)
					return -1;
                if(Upload(error) == -1)
                    return -1;
            }
		}

		return 0;
	}

	// Ask the server show the number of entries in TF and TW.
	int Client::ShowIndexSizes(string &error)
	{
		string data2Send("SHOW");
		data2Send += GAP_STRING;
		if (connIndex.SendData(data2Send, error) == -1)
		{
			error = "Error in sending SHOW message to the server...";
			return -1;
		}

		return 0;
	}

	// Ask the server to terminate.
	int Client::ExitServer(string &error)
	{
		string data2Send("EXIT");
		data2Send += GAP_STRING;
		if (connIndex.SendData(data2Send, error) == -1)
		{
			error = "Error in sending EXIT message to the server...";
			return -1;
		}

		return 0;
	}

	int Client::DeleteFile(string fileName, bool bParallel, string &error)
	{
		if(MF.find(fileName) == MF.end())
		{
			cout << "The file '" << fileName << "' does not exists." << endl;;
			return 0;
		}

        // Delete the corresponding entries from the indexes if it is for forward privacy.
        string FiKey = getFjKey(masterKey, fileName);
        string data2Send;
        data2Send = bParallel ? "PDELETE" : "DELETE" ;
        data2Send += (GAP_STRING + FiKey + to_string(MF[fileName]) + GAP_STRING);
        if (connIndex.SendData(data2Send, error) == -1)
        {
            error = "Error in sending EXIT message to the server...";
            return -1;
        }

		MF.erase(fileName);
		return 0;
	}

	int Client::DeleteBatch(string &error)
	{
		ifstream file(DELETE_FILE_INPUT, std::ifstream::in);
		string line;
		while (getline(file, line))
		{
            cout << "Deleting: " << line << endl;
			std::transform(line.begin(), line.end(), line.begin(), ::toupper);
			if(DeleteFile(line, true, error) == -1)
				return -1;
		}

		return 0;
	}

	int Client::AddNewFile(string fileName, string &error)
	{
		string capName(fileName);
		std::transform(capName.begin(), capName.end(), capName.begin(), ::toupper);
		if(MF.find(capName) != MF.end())
		{
			cout << "The file '" << fileName << "' already exists." << endl;;
			return 0;
		}

		FileReading FR;
		FR.ReadFile("", fileName);
        auto sTime = chrono::system_clock::now();
		FR.BuildIndex(masterKey, TW, TF, MW, MF);
        auto eTime = chrono::system_clock::now();
		writeAddTimeClient(TF.size(), MF.size(), 0);//double(eTime - sTime));
		if (Upload(error) == -1)
			return -1;
		return 0;
	}

	int Client::AddBatch(string &error)
	{
		FileReading FR;
		int sTime = 0, eTime = 0;
		DIR *directoryHandle = opendir("ToBeAdded");
		if (directoryHandle != NULL)
		{
			dirent *entry = readdir(directoryHandle);
			while (entry != NULL)
			{
				if (entry->d_name[0] != '.' && DT_REG == entry->d_type)
					FR.ReadFile("ToBeAdded", entry->d_name);

				sTime = clock();
				FR.BuildIndex(masterKey, TW, TF, MW, MF);
				eTime = clock();
				writeAddTimeClient(TF.size(), MF.size(), double(eTime - sTime));
				if (Upload(error) == -1)
					return -1;
				//go to next entry
				entry = readdir(directoryHandle);
			}

			closedir(directoryHandle);
		}

		return 0;
	}

	void Client::GetKey()
	{
		cout << "masterKey: " << masterKey << endl;
		ofstream ofile;
		ofile.open("key.txt");
		ofile << masterKey;
		ofile.close();
	}

	void Client::SetKey()
	{
		ifstream ifile("key.txt", std::ifstream::in);
		getline(ifile, masterKey);
		cout << "masterKey: " << masterKey << endl;
	}
}
