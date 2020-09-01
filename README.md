

[![DOI](https://zenodo.org/badge/291910887.svg)](https://zenodo.org/badge/latestdoi/291910887)


# Overview

The following section presents a delta run length encoded method. This presented method provided a way to compress a data collected from environmental sensors. Using delta run length encoding provides a method that requires no additional memory to preform the encoding. 

The method presented within this repository operates under the following assumptions
1. Delta (difference) between two data points can be represented as 2's compliment in 13 bits. 


## Delta encoding
Delta encoding takes a input data stream and calculates the difference between the previous value and the current value. Delta encoding requires that a key value (also called initial value) be defined for the encoder and decoder. This implementation of delta encoding uses 0 as the key value.

The delta is defined to be 
```
delta = previousValue - currentValue
```
### Example

**Input Datastream**
```
 1 2 3 5 7 9 7 5 3 2 1
```
There is an implicit 0 prepended to the above data stream. Internally, the delta encoding will see the following datastream.
```
 0 1 2 3 5 7 9 7 5 3 2 1
```

**Output Datastream**
```
-1 -1 -1 -2 -2 -2 2 2 2 1 1
```

## Run Length Encoding
Run length encoding converts a sequence of same values in an input datastream to an output data steam of count value pairs where count is the number of times value occurred in a row. 

### Example

**InputDatastream**
```
-1 -1 -1 -2 -2 -2 2 2 2 1 1
```

**Output Datastream**
```
3 -1 3 -2 3 2 2 1
```


## Delta Run Length Encoding
Delta run length encoding uses the delta and run length encoding methods from above, with the following changes. 
1. MSB of a byte signals if delta is represented in 5 or 13 bits. 
1. 2 bits used to represent count offset by 1 since 0 is an invalid representation in run length encoding. 
1. If delta can fit within 5 bits (0 to 31), then a single byte will encode the byte value pair.


**8 Bit Encoding**

Delta [0,31]
```
7    6      4      0
+------------------+
| 0 | Count | Value|
+------------------+
``` 


**16 Bit Encoding**


```
7    6      4      0
+----------------------+
| 0 | Count | Value MSB|
+----------------------+


7                  0
+------------------+
|    Value LSB     |
+------------------+
``` 

> **NOTE**: Since value is stored as 2's compliment, any negative delta will require 16 bit encoding to be used.


# Algorithm Validation

## Datasets

In order to test the developed delta run length encoded algorithm, data sets for temperature and humidity were obtained from the National Oceanic Atmospheric Administration (NOAA). More specifically the [Sub-hourly quality control datasets](https://www.ncdc.noaa.gov/crn/qcdatasets.html)<sup>[1]</sup><sup>[2]</sup> were used.

Place download datasets into the "OriginalFiles" folder.

## Validation Steps

Validation happens in the following steps
1. Extract required data from NOAA data sets
1. Apply delta run length encoding
1. Decode delta run length encoded data
1. Check decoded data matches input data


## Performance Characterization

After placing all input files into "OriginalFiles", issue ```make report``` from root directory. This will build and run the test application. Upon completion, performance characteristics for the specific input data set will be printed to the terminal.

Doing so with the default dataset of "CRNS0101-05-2019-CA_Yosemite_Village_12_W.out1.txt" should result in the following output in the terminal.

```
======================================
FILENAME: Build/CRNS0101-05-2019-CA_Yosemite_Village_12_W.out1.txt
Raw Number Bytes              : 210060  Space Savings: 0.0000   Compression Ratio: 1.0000
Run Length Encoded            : 233241  Space Savings: -11.0354 Compression Ratio: 0.9006
Delta Run Length Encoded      : 127984  Space Savings: 39.0726  Compression Ratio: 1.6413
Delta Delta Run Length Encoded: 146433  Space Savings: 30.2899  Compression Ratio: 1.4345
NOTE: Percent Compression < 0 means encoded file is larger than original file
======================================
```


# Notes

## HDC1080

Temperature can be converted using the following formula provided on page 14 of the datasheet.


```
Temp (in C) = (rawValue/2^16)*165-40
```

If one would like to derive an expression for the raw ADC value given a temperature in Celsius, the following equation is derived. 

```
((Temp+40)/165)*2^16 = rawValue
```

The above derived formula is used to calculate raw ADC values from the NOAA datasets. The raw ADC value are used by the delta run length encoding algorithm.


# Notes
* DO NOT EDIT FILES IN ORIGINAL FILES FOLDER. This folder should contain ONLY the NOAA obtained data sets in CSV format.

# Footnotes
[1] Diamond, H. J., T. R. Karl, M. A. Palecki, C. B. Baker, J. E. Bell, R. D. Leeper, D. R. Easterling, J. H. Lawrimore, T. P. Meyers, M. R. Helfert, G. Goodge, and P. W. Thorne, 2013: U.S. Climate Reference Network after one decade of operations: status and assessment. Bull. Amer. Meteor. Soc., 94, 489-498.
doi: 10.1175/BAMS-D-12-00170.1

[2] Bell, J. E., M. A. Palecki, C. B. Baker, W. G. Collins, J. H. Lawrimore, R. D. Leeper, M. E. Hall, J. Kochendorfer, T. P. Meyers, T. Wilson, and H. J. Diamond. 2013: U.S. Climate Reference Network soil moisture and temperature observations. J. Hydrometeorol., 14, 977-988.
doi: 10.1175/JHM-D-12-0146.1


# Special Thanks

<table><tr>
<td> <img src="./UCD_EEC.png"  style="width: 250px;"/> </td>
<td> <img src="./MCSG.bmp"  style="width: 250px;"/> </td>
</tr></table>
