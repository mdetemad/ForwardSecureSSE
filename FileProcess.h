#pragma once
#include "Utils.h"

namespace sse
{
    struct BothAreSpaces
    {
        char c;
        BothAreSpaces(char r) : c(r) {}
        bool operator()(char l, char r) const
        {
                return r == c && l == c;
        }
    };

    class FileReading
    {
        private:
            mutex mtx;
            std::map<string, std::vector<string> > file2words, invIndex;
            void ProcessLine(string &line);
            bool IsEnglish(string word);
            void AddKeyword(string masterKey, string FjKey, string keywords, string fileName, SSEMap &TW, SSEMap &TF, MWMap &MW, MFMap &MF);

        public:
            FileReading();
            void ReadIndex(string masterKey, SSEMap &TW, SSEMap &TF, MWMap &MW, MFMap &MF);
            void BuildIndex(string masterKey, SSEMap &TW, SSEMap &TF, MWMap &MW, MFMap &MF);
            void ReadFile(string path, string fileName);
            int  ReadWikiArchives(string &error);
            int  ReadWikiArchive(string &error);
            void ReadFiles(string path);
            void ConvertIndex2Sophos();
            void WriteIndex();
            void BuildInvertedIndex();
            void WriteInvertedIndex();
            void WriteIndexSophos(int nProcessed);
    };
}//namespace
