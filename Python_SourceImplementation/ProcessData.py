#! /bin/python

# Used for operating on text files.
import sys
# Used for checking of a file exists
import os

# Control the maximum allowed number of repetitions.
MAX_RUN_LENGTH = 4


# Convert a value to hexadecimal value that represents the input value. 
# The number of bits controls the total number of output bits that the 
# number will be represented in.
def tohex(val, nbits):
  return ((hex((val + (1 << nbits)) % (1 << nbits))))

# Derived equation for converting a temperature value to a 14 bit binary value. 
# The last& with the constant is used to make sure that the value is 16 bits total and 
# that the low 2 lsbs are 0. The datasheet defines that these two bits are fixed to be 0.
def ConvertToHDC(value):
    return int(((value+(40))/(165)) * (2**16))>>2

def SpaceSavings(uncompressedSize, compressedSize):
    return "{:.4f}".format((1.0 - (compressedSize/uncompressedSize))*100.0)

def CompressionRatio(uncompressedSize, compressedSize):
    return "{:.4f}".format(uncompressedSize/compressedSize)


def GenerateReports(dataFile, runLengthEncodedFile, deltaRunLengthEncodedFile, deltaDeltaRunLengthEncodedFile, reportFile):
    # Opens file with write permission where output will be placed
    with open(reportFile,"w") as out:
        # Define the variables that will be used to store the measured values
        numberRawFileBytes   = 0
        runLengthEncodeBytes = 0
        deltaRunLengthEncodedBytes = 0
        deltaDeltaRunLengthEncodedBytes = 0

        # first calculate the number of entries in the 
        with open(dataFile,"r") as f:
            # Iterate over every line in the input file
            for line in f:
                lineData = line.split()
                # Each entry will be encoded as two bytes (8 bits)
                numberRawFileBytes = numberRawFileBytes + 2

        with open(runLengthEncodedFile,"r") as f:
            # Iterate over every line in the input file
            for line in f:
                # Each entry will be encoded as three bytes (24 bits)
                # 1 byte for number of repetitions
                # 2 bytes for the value.
                runLengthEncodeBytes = runLengthEncodeBytes + 3

        with open(deltaRunLengthEncodedFile,"r") as f:
            # Iterate over every line in the input file
            for line in f:
                # Get the numerical value of the line
                lineData = line.split()
                # RLE data files have the first byte which represents the number of repeats of the value in the following two bytes.
                currLineValue = int(lineData[1])

                # Can the number be represented in 5 bits
                # signed 5 bits range is -16 <= x <= 15
                # Numbers are encoded i the following way 
                # 1 bit used to signal is next byte contains the LSBs of value
                #    1 if next byte should be included
                #    0 value is represented within the low 5 lsb of this byte. 
                # 2 bits signal the number of times that the value should be replicated
                # 5 bits represents a signed 2s compliment value
                if (0 <= currLineValue ) and (currLineValue <= 31):
                    # Number is able to fit within 1 byte. 
                    # add one to the total byte compression stat counter
                    deltaRunLengthEncodedBytes = deltaRunLengthEncodedBytes + 1
                else:
                    # Two bytes required to represent the count and value. 
                    deltaRunLengthEncodedBytes = deltaRunLengthEncodedBytes + 2

        with open(deltaDeltaRunLengthEncodedFile,"r") as f:
            # Iterate over every line in the input file
            for line in f:
                # Get the numerical value of the line
                # RLE data files have the first byte which represents the number of repeats of the value in the following two bytes.
                currLineValue = int(line.split()[1])
                # Can the number be represented in 5 bits
                # signed 5 bits range is -16 <= x <= 15
                # Numbers are encoded i the following way 
                # 1 bit used to signal is next byte contains the LSBs of value
                #    1 if next byte should be included
                #    0 value is represented within the low 5 lsb of this byte. 
                # 2 bits signal the number of times that the value should be replicated
                # 5 bits represents a signed 2s compliment value
                if (0 <= currLineValue ) and (currLineValue <= 31):
                    # Number is able to fit within 1 byte. 
                    # add one to the total byte compression stat counter
                    deltaDeltaRunLengthEncodedBytes = deltaDeltaRunLengthEncodedBytes + 1
                else:
                    # Two bytes required to represent the count and value. 
                    deltaDeltaRunLengthEncodedBytes = deltaDeltaRunLengthEncodedBytes + 2

        # Populate the output report file. The following lines will be dumped into the 
        # resulting report file. The following lines summarize the performance of 
        # using the different encoding methods. 
        out.write("======================================\n")
        out.write("FILENAME: " + sys.argv[1] +"\n")
        out.write("Raw Number Bytes              : " + str(numberRawFileBytes)              + "\tSpace Savings: " + SpaceSavings(numberRawFileBytes, numberRawFileBytes)              + "\tCompression Ratio: " + CompressionRatio(numberRawFileBytes, numberRawFileBytes) + "\n")
        out.write("Run Length Encoded            : " + str(runLengthEncodeBytes)            + "\tSpace Savings: " + SpaceSavings(numberRawFileBytes, runLengthEncodeBytes)            + "\tCompression Ratio: " + CompressionRatio(numberRawFileBytes, runLengthEncodeBytes) + "\n")
        out.write("Delta Run Length Encoded      : " + str(deltaRunLengthEncodedBytes)      + "\tSpace Savings: " + SpaceSavings(numberRawFileBytes, deltaRunLengthEncodedBytes)      + "\tCompression Ratio: " + CompressionRatio(numberRawFileBytes, deltaRunLengthEncodedBytes) + "\n")
        out.write("Delta Delta Run Length Encoded: " + str(deltaDeltaRunLengthEncodedBytes) + "\tSpace Savings: " + SpaceSavings(numberRawFileBytes, deltaDeltaRunLengthEncodedBytes) + "\tCompression Ratio: " + CompressionRatio(numberRawFileBytes, deltaDeltaRunLengthEncodedBytes) + "\n")
        out.write("NOTE: Percent Compression < 0 means encoded file is larger than original file\n")
        out.write("======================================")
        out.write("\n\n")






# extracts column of interest and removes all invalid values
def PreProcessFile(InputFile, OutputFile):
    # Opens file with write permission where output will be placed
    with open(OutputFile,"w") as out:
        # Open raw data input file with read permissions.
        with open(InputFile,"r") as f:
            for line in f:
                # Split the line. This is the same behavior as descried above. 
                lineData = line.split()
                value = float(lineData[2])
                # If the temp is above or below 100 degrees (not physically possible on the surface of the earth where the weather stations are), 
                # the value will be discarded. NOAA will set the temp reading to 999 or -999 if the sensor did nit take a valid reading. 
                if ((-100) <= value) and (value <= (100)):
                    out.write("{0}\n".format(ConvertToHDC(value)))



def DeltaEncode(InputFile, OutputFile):
    # Open the temp file in write mode. 
    # The following section will generate an delta encoded output stream of the input stream.
    with open(OutputFile,"w") as out:
        # Open the file specified on the command line.
        # File will be opened in read mode only.
        with open(InputFile,"r") as f:
            # Previous data measurement
            prevLineValue = 0
            # Current data measurement
            currLineValue = 0
            # Iterate over every line in the input file
            for line in f:
                # Split input data by whitespace
                lineData = line.split()

                # Set the current measurement value to be the newly read in measurement.
                ################ COMMENT #######################
                currLineValue = int(lineData[0])

                # The new measurement is valid. 
                # Write to the output file the delta encoded value, both in integer form and the 16 bit signed hex value as well. 
                out.write("{0}\n".format(prevLineValue - currLineValue))
                # Update Previous price to he current price.
                prevLineValue = currLineValue


def RunLengthEncode(InputFile, OutputFile,maxRunLength):
    # Apply run length encoding (RLE) to an input file
    # Open file to where the output will be placed
    with open(OutputFile,"w") as out:
        # REad the file generated during the previous stage (delta encoding)
        with open(InputFile, "r") as f:
            # Set the default values for variables that are used throughout 
            # the RLE procedure.
            prevLineValue = 0
            currLineValue = 0
            # Count represents the number of consecutive values which were the same. 
            # Default to 1 since the first value is assumed to be 0 and there is on 1 prepended 0.
            # Count can only have a value between 0 to 255 since count  must fit in a 8 bit number. 
            count = 1

            # Iterate over every line of the delta encoded file. 
            for line in f:
                # Current delta encoded value is the first element of the line. 
                lineData = line.split()
                ################ COMMENT #######################
                currLineValue = int(lineData[0])
                # Increment count only if current value read matches previous value, 
                # and that there has not been a maxRunLength of the same characters yet.
                if((prevLineValue == currLineValue) and (count <= maxRunLength)):
                    # Increments the count.
                    count = count + 1
                # Either the current value is different or a run of over maxRunLength values was encountered.
                else:
                    # Write the data to the output file. First argument is the count value, followed 
                    # by the previous read data value. 
                    out.write("{0} {1}\n".format(count,prevLineValue))
                    # Reset count to 1. 
                    count = 0
                # Set previous value to current value
                prevLineValue = currLineValue
            # Dump any remaining value to the output file. 
            out.write("{0} {1}\n".format(count,prevLineValue))


# Create variables representing the files that will be used.
# Input data file contains multiple columns of data. 
rawDataFile = sys.argv[1]
# processed rawDataFile that now only includes temp data
dataFile = os.path.splitext(sys.argv[1])[0] + '.data'
# File name for just run length encoded
runLengthEncodedFile      = os.path.splitext(sys.argv[1])[0] + '.rle'
# Delta and run length encoded data filename
deltaEncodedFile =  os.path.splitext(sys.argv[1])[0] + '.d'
# Delta and run length encoded data filename
deltaRunLengthEncodedFile =  os.path.splitext(sys.argv[1])[0] + '.drle'
# Delta encoding twice and run length encoded data once  filename
deltaDeltaRunLengthEncodedFile =  os.path.splitext(sys.argv[1])[0] + '.ddrle'

# Temp file 3 used for an intermediate step
deltaDeltaEncodedFile = os.path.splitext(sys.argv[1])[0] + '.dd'
# Statistics report file name
reportFile =   sys.argv[2]


# Remove all unneeded columns. Just keeps the 3rd column which is the temperature.
PreProcessFile(rawDataFile, dataFile)
# Just perform run length encoding on the input data file
RunLengthEncode(dataFile, runLengthEncodedFile, 255)
# Delta encode input data
DeltaEncode(dataFile, deltaEncodedFile)
# Perform run length encoding on delta encoded data.
RunLengthEncode(deltaEncodedFile, deltaRunLengthEncodedFile, MAX_RUN_LENGTH)

# Delta encode input data
DeltaEncode(deltaEncodedFile,deltaDeltaEncodedFile)
# Perform run length encoding on input data.
RunLengthEncode(deltaDeltaEncodedFile, deltaDeltaRunLengthEncodedFile, MAX_RUN_LENGTH)

# Calculates statistics for compression methods.
GenerateReports(dataFile, runLengthEncodedFile, deltaRunLengthEncodedFile, deltaDeltaRunLengthEncodedFile, reportFile)