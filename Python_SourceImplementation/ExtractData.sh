#! /bin/bash

# Just prints a message to the shell that data is being extracted and placed somewhere.
echo "Extracting Data From" $1 "to " $2

# Output file in format UTC DATE, UTC TIME, Celsius, 16 bit signed hex
# See the NOAA specified header format. This is provided by NOAA in HEADER.txt
#
# See README.md to see th derivation of the 16 bit hex value.
cat $1 | awk '{ printf("%d %04d %f %f %04X\n"), $2, $3, $9, $9, int((($9+40)/165)*(2^16))}' > $2
