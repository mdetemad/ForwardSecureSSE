#include "Connection.h"

namespace sse
{
    Connection::Connection(int portNo, string IP)
    {
        serverPort = portNo;
        IPAddr = IP;
    }

    int Connection::Connect(string &error)
    {
        hostInfo = gethostbyname(IPAddr.c_str());
        if (hostInfo == NULL) 
        {
            error = "Host undefined...";
            return -1;
        }

        // Now, create a socket for data transmission. 
        socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
        if (socketDescriptor < 0) 
        {
            error = "Cannot create socket ...";
            return -1;
        }

        // Now, connect to server.
        serverAddress.sin_family = hostInfo->h_addrtype;
        memcpy((char *) &serverAddress.sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
        serverAddress.sin_port = htons(serverPort);

        if (connect(socketDescriptor, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
        {
            error = "Error in connection ...";
            return -1;
        }
        
        return 0;
    }
    
    int Connection::SendData(string data, string &error)
    {
        char data2Send[data.size()];
        str2Char(data, data2Send);
        //cout << "data2Send: " << data.size() << endl;
        if (send(socketDescriptor, data2Send, data.size(), 0) < 0) 
        {
            error = "Error in data transmission...";
            close(socketDescriptor);
            return -1;
        }
        
        return 0;
    }
    
    void Connection::Close()
    {
          close(socketDescriptor);
    }
    
}//namespace