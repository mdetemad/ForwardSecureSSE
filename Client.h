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
#include "Connection.h"
#include "FileProcess.h"
#include <unistd.h>
#include "dirent.h"
#include "pugixml.hpp"
#include "porter2_stemmer.h"

using namespace pugi; 

namespace sse
{
    struct wordBox
    {
        string word;
        int noWords, noFiles;
    };

	class Client
	{
		private:
			Connection connIndex;
			string error, masterKey;
			char buffer[BUFF_LENGTH];
			SSEMap TW, TF;
			MWMap MW;
			MFMap MF;
            mutex mtx;
            SSEType sseType;
            int minNoise, maxNoise;

            void ReadFromIndex(int &nPairs);
            int AddKeyword(string FjKey, string fileName, vector<wordBox> wb, int &totalSent);
		
		public:
			Client(int serverPort, const string serverAddr);
			int Connect(string &error);
            int AddNoise(string &error);
			int PreProcess(string &error);
			int Upload(string &error);
			int Search(string keyword, int &fileNo, SearchType opType, string &error);
			int SearchBatch(string &error);
			int ParallelSearchBatch(string &error);
			int ShowIndexSizes(string &error);
			int ExitServer(string &error);
			int DeleteFile(string fileName, bool bParallel, string &error);
			int DeleteBatch(string &error);
			int AddNewFile(string fileName, string &error);
			int AddBatch(string &error);
            void SetType(SSEType isseType);
            void ComputeMax();
			void GetKey();
			void SetKey();
			void Close();
    };	
}