/*
 Forward- and Backeard-private Searchable Symmetric Encryption
 Copyright (C) 2016 Mohammad Etemad

 This is a free software: you can redistribute/modify it 
 under the GNU Affero General Public License.

 This is distributed freely hoping to help further improvements in the
 searchable encryption area, as is and WITHOUT ANY WARRANTY.
 See the GNU Affero General Public License for more details.
 
*/

#include "Utils.h"

namespace sse
{
    class Server
    {
        private:
            int serverSocket, connSocket, portNo, completed;
            socklen_t cAddressLength;
            struct sockaddr_in cAddress, sAddress;
            char buffer[BUFF_LENGTH], readTill[4*KEY_LENGTH];
            string command, setupType, searchKey, myAddr;
            SSEMap TW, TF;
            std::vector<string> recPairs;
            mutex mtx;
            int DeletePart(int start, int end);
            int SearchPart(int start, int end);
            int Connect(string &error);
            void Build(string readBuff);
            void Search(string readBuff);
            void Delete(string readBuff);

        public:
            Server(int port, const string imyAddr);
            int Start();
    };
}
