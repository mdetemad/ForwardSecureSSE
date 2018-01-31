#pragma once
#include "Utils.h"

namespace sse
{
    string getMasterKey(int keyLength);
    string getFjKey(string key, string Fj);
    string getWiKey(string key, string Wi, int searchNo);

    class Connection
    {
        private:
        unsigned short int serverPort;
        string IPAddr;
        struct sockaddr_in serverAddress;
        struct hostent *hostInfo;
        
        public:        
        int socketDescriptor;
        Connection(int portNo, string IP);
        int Connect(string &error);
        int SendData(string data, string &error);
        void Close();
    };
    
    
}//namespace