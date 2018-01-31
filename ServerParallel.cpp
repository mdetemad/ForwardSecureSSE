#include "Server.h"

namespace sse
{
    Server::Server(int port, const string imyAddr)
    {
        portNo = port;
        myAddr = imyAddr;
    }

    int Server::Connect(string &error)
    {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) 
        {
            error = "Error in listening socket creation...";
            return -1;
        }

        sAddress.sin_family = AF_INET;
        sAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        sAddress.sin_port = htons(portNo);

        if(bind(serverSocket, (sockaddr *) &sAddress, sizeof(sAddress)) < 0) 
        {
            error = "Error in binding socket...";
            return -1;
        }

        // Wait for connections from clients.
        listen(serverSocket, 5);
        return 0;
    }

    void Server::Build(string readBuff)
    {
        //cout << "Inside build, setupType: " << setupType << "\treadBuff: " << readBuff << endl;
        if(readBuff == "TF")
            setupType = "TF";
        else if(readBuff == "TW")
            setupType = "TW";
        else
        {
            if(setupType == "TF")
                TF[readBuff.substr(0, KEY_LENGTH)] = readBuff.substr(KEY_LENGTH);
            else if(setupType == "TW")
                TW[readBuff.substr(0, KEY_LENGTH)] = readBuff.substr(KEY_LENGTH);
        }
    }

    void Server::Search(string readBuff)
    {
        recPairs.clear();
        char dataChar[BUFF_LENGTH];
        string data2Send, address, mask, WiKey = readBuff.substr(0, KEY_LENGTH);
        int i = 0, fileNo = stoi(readBuff.substr(KEY_LENGTH));
        clock_t eTime = 0, sTime = clock(), allTime = 0;
        cout << "fileNo: " << fileNo << endl;
        for(int cnt = 0 ; cnt < fileNo; cnt++)
        {
            address = getWiAddress(WiKey, cnt);
            if(TW.find(address) == TW.end())
            {
                string entry(DEL_STRING);
                recPairs.push_back(entry);
            }else
            {
                mask = getWiMask(WiKey, cnt);
                if(TW[address].length() > KEY_LENGTH)
                    for(i = KEY_LENGTH; i < TW[address].length(); i++)
                        TW[address][i] = TW[address][i]^mask[i - KEY_LENGTH];
                string entry(TW[address]);
                recPairs.push_back(entry);
                TW.erase(address);
            }
        }

        eTime = clock();
        writeSearchTimeServer(fileNo, TW.size(), double(eTime - sTime), OPType::regular);
        for (std::vector<string>::iterator it = recPairs.begin(); it != recPairs.end(); ++it)
        {
            if(data2Send.size() + (*it).size() + sizeof(GAP_STRING) > BUFF_LENGTH)
            {
                str2Char(data2Send, dataChar);
                //cout << "data2Send: " << data2Send.size() << endl;
                if (send(connSocket, dataChar, data2Send.size(), 0) < 0)
                {
                    cout << "Error in sending search result" << endl;
                    return;
                }

                data2Send = "";
            }
            
            data2Send += ((*it) + GAP_STRING);
        }

        data2Send += END_STRING;
        data2Send += GAP_STRING;
        str2Char(data2Send, dataChar);
        if (send(connSocket, dataChar, data2Send.size(), 0) < 0)
        {
            cout << "Error in sending search result";
            return;
        }
    }

    void Server::SearchPart(int start, int end)
    {
        string address, mask;
        for(int cnt = start; cnt < end; cnt++)
        {
            address = getWiAddress(searchKey, cnt);
            if(TW.find(address) == TW.end())
            {
                string entry(DEL_STRING);
                recPairs.push_back(entry);
            }else
            {
                mask = getWiMask(searchKey, cnt);
                if(TW[address].length() > KEY_LENGTH)
                    for(int i = KEY_LENGTH; i < TW[address].length(); i++)
                        TW[address][i] = TW[address][i]^mask[i - KEY_LENGTH];
                string entry(TW[address]);
                mtx.lock();
                recPairs.push_back(entry);
                mtx.unlock();
                TW.erase(address);
            }
        }
    }

    void Server::SearchParallel(string readBuff)
    {
        searchKey = readBuff.substr(0, KEY_LENGTH);
        int fileNo = stoi(readBuff.substr(KEY_LENGTH));
        cout << "fileNo: " << fileNo << endl;
        clock_t eTime = 0, sTime = clock();

        thread t1(&Server::SearchPart, this, 0, fileNo/4);
        thread t2(&Server::SearchPart, this, fileNo/4, fileNo/2);
        thread t3(&Server::SearchPart, this, fileNo/2, 3*fileNo/4);
        thread t4(&Server::SearchPart, this, 3*fileNo/4, fileNo);
        t1.join();
        t2.join();
        t3.join();
        t4.join();

        eTime = clock();
        writeSearchTimeServer(fileNo, TW.size(), double(eTime - sTime), OPType::parallel);
        char dataChar[BUFF_LENGTH];
        string data2Send = "";
        for (std::vector<string>::iterator it = recPairs.begin(); it != recPairs.end(); ++it)
        {
            if(data2Send.size() + (*it).size() + sizeof(GAP_STRING) > BUFF_LENGTH)
            {
                str2Char(data2Send, dataChar);
                //cout << "data2Send: " << data2Send.size() << endl;
                if (send(connSocket, dataChar, data2Send.size(), 0) < 0)
                {
                    cout << "Error in sending search result" << endl;
                    return;
                }

                data2Send = "";
            }
            
            data2Send += ((*it) + GAP_STRING);
            //cout << "Data: " << *it << endl;
        }

        recPairs.clear();
        data2Send += END_STRING;
        data2Send += GAP_STRING;
        str2Char(data2Send, dataChar);
        if (send(connSocket, dataChar, data2Send.size(), 0) < 0)
        {
            cout << "Error in sending search result";
            return;
        }
    }

    void Server::Delete(string readBuff)
    {
        int sTime = clock();
        char fName[2*KEY_LENGTH], dataChar[BUFF_LENGTH];
        string address, FjKey = readBuff.substr(0, KEY_LENGTH);
        int wordNo = stoi(readBuff.substr(KEY_LENGTH));
        cout << "wordNo: " << wordNo << endl;
        for(int cnt = 0 ; cnt < wordNo; cnt++)
        {
            address = getFjAddress(FjKey, cnt);
            TW.erase(TF[address]);
            TF.erase(address);
/*            if(TF.find(address) != TF.end())
            {
                if(TW.find(TF[address]) != TW.end())
                    TW.erase(TF[address]);
                else
                    cout << "Error in deletion (TW) ..." << endl;;
                TF.erase(address);
            }else
                cout << "Error in deletion (TF) ..." << endl;;
                //cout << "cnt: " << cnt << "\t\tsize: " << address.size() << "\t\taddress: " << address << endl;
*/        }

        int eTime = clock();
        writeDelTimeServer(wordNo, TF.size() + wordNo, double(eTime - sTime), OPType::regular);
    }

    void Server::DeletePart(int start, int end)
    {
        string address;
        clock_t sTime = clock();
        for(int cnt = start ; cnt < end; cnt++)
        {
            address = getFjAddress(searchKey, cnt);
            TW.erase(TF[address]);
            TF.erase(address);
/*            if(TF.find(address) != TF.end())
            {
                if(TW.find(TF[address]) != TW.end())
                    TW.erase(TF[address]);
                else
                    cout << "Error in deletion (TW) ..." << endl;;
                TF.erase(address);
            }else
                cout << "Error in deletion (TF) ..." << endl;;
                cout << "cnt: " << cnt << "\t\tsize: " << address.size() << "\t\taddress: " << address << endl;
*/        }

        int eTime = clock();
        writeDelTimeServer(4*(end - start), TF.size() + 4*(end - start), double(eTime - sTime), OPType::parallel);
    }

    void Server::DeleteParallel(string readBuff)
    {
        char fName[2*KEY_LENGTH], dataChar[BUFF_LENGTH];
        searchKey = readBuff.substr(0, KEY_LENGTH);
        int wordNo = stoi(readBuff.substr(KEY_LENGTH));
        cout << "wordNo: " << wordNo << endl;

        thread t1(&Server::DeletePart, this, 0, wordNo/4);
        thread t2(&Server::DeletePart, this, wordNo/4, wordNo/2);
        thread t3(&Server::DeletePart, this, wordNo/2, 3*wordNo/4);
        thread t4(&Server::DeletePart, this, 3*wordNo/4, wordNo);
        t1.join();
        t2.join();
        t3.join();
        t4.join();
    }

    int Server::Start()
    {
        string error, strNull("NULL");
        int nRead = 0, nReceived = 0;
        if(Connect(error) < 0)
        {
            cout << error << endl;
            return -1;
        }

        TW.set_deleted_key(strNull);
        TF.set_deleted_key(strNull);
        while (true) 
        {
            cAddressLength = sizeof(cAddress);
            connSocket = accept(serverSocket, (struct sockaddr *) &cAddress, &cAddressLength);
            if (connSocket < 0) 
            {
                cout << error << endl;
                return -1;
            }

            while ((nReceived = recv(connSocket, buffer, BUFF_LENGTH, 0)) > 0) 
            {
                int nCur = 0;
                //cout << "nReceived: " << nReceived << endl;
                while(nCur < nReceived)
                {
                    readTill[nRead++] = buffer[nCur++];
                    //cout << "nCur: " << nCur << "\tnRead: " << nRead << "\t" << buffer[nCur - 1] << endl;

                    // If end of segment of data reached
                    if(nRead > 4 && readTill[nRead - 1] == 'E' && readTill[nRead - 2] == 'D' && readTill[nRead - 3] == 'C' && readTill[nRead - 4] == 'B' && readTill[nRead - 5] == 'A')
                    {
                        //cout << "nCur: " << nCur << "\tnRecv: " << nReceived << "\t\tnRead: " << nRead << endl;
                        nRead = nRead - 5;
                        string buff;
                        char2Str(readTill, buff, nRead);
                        nRead = 0;
                        if(buff == "BUILD")
                            command = "BUILD";
                        else if(buff == "SEARCH")
                            command = "SEARCH";
                        else if(buff == "PSEARCH")
                            command = "PSEARCH";
                        else if(buff == "DELETE")
                            command = "DELETE";
                        else if(buff == "PDELETE")
                            command = "PDELETE";
                        else if(buff == "SHOW")
                            cout << "TW size: " << TW.size() << "\tTF size: " << TF.size() << endl;
                        else if(buff == "EXIT")
                            exit(0);
                        else
                        {
                            //cout << "Before calling\tcommand:" << command << "\tbuff:" << buff << endl;
                            if(command == "BUILD")
                                Build(buff);
                            else if(command == "SEARCH")
                                Search(buff);
                            else if(command == "PSEARCH")
                                SearchParallel(buff);
                            else if(command == "DELETE")
                                Delete(buff);
                            else if(command == "PDELETE")
                                DeleteParallel(buff);
                        }

                        std::string().swap(buff);
                    }
                }
            }
        }
    }
}