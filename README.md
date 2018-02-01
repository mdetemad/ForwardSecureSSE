# ForwardSecureSSE
This is an implementation of a forward-secure searchable encryption scheme in C++.

This project contains two parts: the server and the client. Once compiled and ran on the respective machines, they communicate through TCP/IP. The client generates and upload the index on the server. Later on, the server answers client's queries.
Once the client extracts the searchable keywords and builds the index, the files will not be used anymore. I.e., we generate the index from the real files, however, as the aim of this project is to measure the performance of operations on the index, the files themselves are not transferred. (Encrypting and transferring the files would be similar among all SSE schemes.)

# Required packages
We use crypto++ version 5.6.5 for cryptographic operations.

# Compilation
These two parts should be compiled separately.  For ease, two executable files mserver and mclinet are provided that would do the compilation.

# Input format
The client accepts index files of the following format:

File: fileName

Keyword1 Keyword2 Keyword3 Keyword4 Keyword5 ...

I.e., all searchable unique keywords of each file are stored next to the file name.
We have also provided a possibility to read all files in a given folder and convert them all into this format.
