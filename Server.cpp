#include "Server.h"

namespace sse
{
    Server::Server(int port, const string imyAddr)
    {
        portNo = port;
        myAddr = imyAddr;
        completed = 1;
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

    int Server::SearchPart(int start, int end)
    {
        char dataChar[BUFF_LENGTH];
        string data2Send, address, mask;
        for(int cnt = start; cnt < end; cnt++)
        {
            address = getWiAddress(searchKey, cnt);
            string TWData = TW[address];
            TW.erase(address);
            if(data2Send.size() + TWData.size() + sizeof(GAP_STRING) > BUFF_LENGTH)
            {
                str2Char(data2Send, dataChar);
                //cout << "data2Send: " << data2Send.size() << endl;
                if (send(connSocket, dataChar, data2Send.size(), 0) < 0)
                {
                    cout << "Error in sending search result" << endl;
                    return -1;
                }

                data2Send = "";
            }

            mask = getWiMask(searchKey, cnt);
            for(int i = KEY_LENGTH; i < TWData.size(); i++)
                TWData[i] = TWData[i]^mask[i - KEY_LENGTH];
            data2Send += (TWData + GAP_STRING);
            //cout << "TWData: " << TWData << endl;
        }

        mtx.lock();
        if(completed == PARALLEL_NO)
        {
            data2Send += END_STRING;
            data2Send += GAP_STRING;
        }else
            completed++;
        mtx.unlock();

        str2Char(data2Send, dataChar);
        return send(connSocket, dataChar, data2Send.size(), 0);
    }

    void Server::Search(string readBuff)
    {
        searchKey = readBuff.substr(0, KEY_LENGTH);
        int fileNo = stoi(readBuff.substr(KEY_LENGTH));
        cout << "fileNo: " << fileNo << endl;

        if(command == "SEARCH")
        {
            completed = PARALLEL_NO;
            auto sTime = chrono::system_clock::now();
            auto t = async(launch::async, &Server::SearchPart, this, 0, fileNo);
            t.get();
            auto eTime = chrono::system_clock::now();
            writeSearchTimeServer(fileNo, TW.size(), CHRONO(sTime, eTime), SearchType::regular);
        }else
        {
            completed = 1;
            vector<future<int>> futures;
            auto sTime = chrono::system_clock::now();
            for(int i = 0; i < PARALLEL_NO; ++i)
                futures.push_back(async(launch::async, &Server::SearchPart, this, i*fileNo/PARALLEL_NO, (i+1)*fileNo/PARALLEL_NO));
            for(int i = 0; i < futures.size(); i++)
                futures[i].get();
            auto eTime = chrono::system_clock::now();
            writeSearchTimeServer(fileNo, TW.size(), CHRONO(sTime, eTime), SearchType::parallel);
        }
    }

    void Server::Delete(string readBuff)
    {
        clock_t sTime = clock();
        char fName[2*KEY_LENGTH], dataChar[BUFF_LENGTH];
        string address, FjKey = readBuff.substr(0, KEY_LENGTH);
        int wordNo = stoi(readBuff.substr(KEY_LENGTH));
        cout << "wordNo: " << wordNo << endl;
        if(command == "DELETE")
        {
            auto sTime = chrono::system_clock::now();
            DeletePart(0, wordNo);
            auto eTime = chrono::system_clock::now();
            writeDelTimeServer(wordNo, TF.size() + wordNo, CHRONO(sTime, eTime), SearchType::regular);
        }else
        {
            vector<future<int>> futures;
            auto sTime = chrono::system_clock::now();
            for(int i = 0; i < PARALLEL_NO; ++i)
                futures.push_back(async(launch::async, &Server::DeletePart, this, i*wordNo/PARALLEL_NO, (i+1)*wordNo/PARALLEL_NO));
            for(int i = 0; i < futures.size(); i++)
                futures[i].get();
            auto eTime = chrono::system_clock::now();
            writeDelTimeServer(wordNo, TF.size() + wordNo, CHRONO(sTime, eTime), SearchType::parallel);
        }
    }

    int Server::DeletePart(int start, int end)
    {
        string address;
        clock_t sTime = clock();
        for(int cnt = start ; cnt < end; cnt++)
        {
            address = getFjAddress(searchKey, cnt);
            TW.erase(TF[address]);
            TF.erase(address);
        }

        return 0;
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
                        if(buff == "BUILD" || buff == "SEARCH" || buff == "PSEARCH" || buff == "DELETE" || buff == "PDELETE")
                            command = buff;
                        else if(buff == "SHOW")
                            cout << "TW size: " << TW.size() << "\tTF size: " << TF.size() << endl;
                        else if(buff == "EXIT")
                            exit(0);
                        else
                        {
                            if(command == "BUILD")
                                Build(buff);
                            else if(command == "SEARCH" || command == "PSEARCH")
                                Search(buff);
                            else if(command == "DELETE" || command == "PDELETE")
                                Delete(buff);
                        }

                        std::string().swap(buff);
                    }
                }
            }
        }
    }
}
