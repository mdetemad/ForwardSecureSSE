#include "Server.h"

using namespace std;
using namespace sse;

int main()
{
    int serverPort = 2324;
    string error;
	Server server(serverPort, "");
    server.Start();
}
