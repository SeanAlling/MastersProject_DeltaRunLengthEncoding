#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <bitset>
#include "encoding.h"
#include "decoding.h"
using namespace std;

vector<uint16_t> inputData;
vector<uint16_t> decompressedData;
vector<uint8_t> buffer;

void WriteToTerminal_DRLE(uint8_t data)
{
    /*
        Places all data into a internal buffer.
    */
    buffer.push_back(data);
}

void WriteToTerminal_DecodeDRLE(uint16_t data)
{
    decompressedData.push_back(data);
}



/*
    Main will take two arguments 
        1. Input filename


        Input file will contain two values per line, 
        the first value is a integer, and the second a hex value.
        The hex value will be used to for being encoded. 

        This program is simple, it simply reads in the hex value into an internal 
        buffer that will be passed to the function.

*/
int main(int argc, char* argv[])
{

    /* One argument expected. If not, the return error */
    if(argc < 2)
    {
        printf("ERROR: Exactly 1 or more arguments (paths input files)");
        return -1;
    }

    for(int index = 1; index < argc; index++)
    {
        /* Open file that is passed as an argument */
        fstream inputFile;
        inputFile.open(argv[index], ios::in);
        /* delete contents of vector */
        inputData.clear();
        buffer.clear();
        //checking whether the file is open
        if (inputFile.is_open())
        {
            /* Variable where input data will be placed */
            int16_t data;

            /* Iterate over contents of input file. Read the values into the internal vector */
            while(inputFile >> data)
            {
                /* Store the read in data in a vector for later processing */
               inputData.push_back(data);
            }
             //close the file object.
            inputFile.close();
        }
        /* Input file not open. This is an error. Return nonzero to signal error */
        else
        {
            printf("ERROR: Can't open input file specified in argv[1]");
            return -1;
        }


        EncodeData_DRLE((inputData.data()), inputData.size(), WriteToTerminal_DRLE);

    /* 
         The following block is used to test the encoder and decoder. If debug is enabled
        the  compressed data stream will be decompressed and compared against the input 
        data stream. PASS or FAILis reported.
    */
    #if defined(DEBUG)
        decompressedData.clear();
        printf("============================================\n");
        DecodeData_DRLE(buffer.data(), buffer.size(), WriteToTerminal_DecodeDRLE);
        printf("Decompressed Data = Input Data:  ");
        if(decompressedData == inputData)
        {
            printf("PASS\n");
        }
        else
        {
            printf("FAIL\n");
        }
        printf("============================================\n");
        printf("\n\n");
    #endif
        /* 
            Size is multiplied by 2 since values are 2 bytes 

            Te next two lines are multiplied by a floating point value to also convert the 
            integer to floating point. This is done for making the calculation later 
            be represented as decimal instead of just integers.
        */
        float uncompressedSize = inputData.size()*2.0;
        float compressedSize   = buffer.size()*1.0;
        /*
           The following section prints some statistics to the screen on the compression
           of the input file.

           The main items of interest is the compression ratio and space savings. 
        */
        printf("============================================\n");
        printf("Delta Run Length Encoded\n");
        printf("\n");
        printf("Input Bytes: %u\tCompressedBytes %u\n" , (unsigned int)(inputData.size()*2), (unsigned int)buffer.size());
        printf("Space Savings %.2f%%\n"                 , (1.0 - (compressedSize/uncompressedSize))*100.0);
        printf("Compression Ratio %.2f\n"              , uncompressedSize/compressedSize);
        printf("============================================\n");
        printf("\n");
    }
}