
#include "Client.h"

using namespace sse;

int showMenu(Client &client, string &error)
{
	char option;
	string input;
	do
	{
		//Displaying Options for the menu
		cout << endl << endl;
//        cout << "\t\tB)uild Index" << endl;
		cout << "\t\tP)reprocess" << endl;
		cout << "\t\tS)earch" << endl;
		cout << "\t\tPaR)allel search" << endl;
		cout << "\t\tA)dd a new file" << endl;
		cout << "\t\tD)elete a file" << endl;
		cout << "\t\tParalleL) delete" << endl;
		cout << "\t\tsH)ow index size" << endl;
		cout << "\t\tE)xit" << endl;
//		cout << "\t\tSet K)ey" << endl;
//        cout << "\t\tG)et Key" << endl;

		cout << "Please select an option : ";
		cin >> option;

		switch (option)
		{
/*			case 'b':
			case 'B':
				if(client.BuildWikiIndexFile(error) == -1)
                    return -1;
				break;
*/				
			case 'p':
			case 'P':
				if(client.PreProcess(error) == -1)
                    return -1;
				//if (client.Upload(error) == -1) 
					//return -1;
				break;
				
			case 's':
			case 'S':
				//cout << "Enter the keyword to search for: ";
				//cin >> input;
				//std::transform(input.begin(), input.end(), input.begin(), ::toupper);
				//if (client.Search(input, error) == -1) 
				if (client.SearchBatch(error) == -1) 
					return -1;
				break;
				
			case 'r':
			case 'R':
				//cout << "Enter the keyword to search for: ";
				//cin >> input;
				//std::transform(input.begin(), input.end(), input.begin(), ::toupper);
				//if (ParallelSearch(input, error) == -1) 
				if (client.ParallelSearchBatch(error) == -1) 
					return -1;
				break;
				
			case 'h':
			case 'H':
				client.ShowIndexSizes(error);
				break;

			case 'a':
			case 'A':    
				//cout << "Enter the file name: ";
				//cin >> input;
				//if(AddNewFile(input, error) == -1)
					//return -1;
				if(client.AddBatch(error) == -1)
					return -1;
				break;

			case 'd':
			case 'D':    
				cout << "Enter the file name: ";
				cin >> input;
				std::transform(input.begin(), input.end(), input.begin(), ::toupper);
				if(client.DeleteFile(input, true, error) == -1)
					return -1;
				break;

			case 'l':
			case 'L':    
				//cout << "Enter the file name: ";
				//cin >> input;
				//std::transform(input.begin(), input.end(), input.begin(), ::toupper);
				//if(client.DeleteFile(input, true, error) == -1)
				if(client.DeleteBatch(error) == -1)
					return -1;
				break;

			case 'g':
			case 'G':    
				client.GetKey();
				break;

			case 'k':
			case 'K':    
				client.SetKey();
				break;
		}
	}while (option != 'E' && option != 'e');
	
	client.ExitServer(error);
	return 0;
}

int main(int argc, char* argv[])
{
    if(argc != 2)
	{
		cout << "Usage: " << argv[0] << " Forward | Backward" << endl;
        return -1;
	}
    
	int serverPort = 2324;
	const string serverAddr("52.26.180.195");
    string error, forback(argv[1]);
	Client client(serverPort, serverAddr);
    if(!forback.compare("Forward") || !forback.compare("forward"))
        client.SetType(SSEType::forward);
    else if(!forback.compare("Backward") || !forback.compare("backward"))
        client.SetType(SSEType::backward);
    else
	{
		cout << "Usage: " << argv[0] << " Forward | Backward" << endl;
        return -1;
	}
        

    if (client.Connect(error) == -1) 
    {
        cout << error << endl;
        return -1;
    }

    if (showMenu(client, error) == -1) 
    {
        cout << error << endl;
        return -1;
    }

    client.Close();
    return 0;
}
