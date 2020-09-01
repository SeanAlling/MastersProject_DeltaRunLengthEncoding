/*
    This file contains the definitions for three functions, 
        
        TransmiteData_DRLE
        TransmiteData_DELTA
        TransmiteData_RLE

    The purpose of these functions is to perform some kind of encoding. The DRLE function
    takes a data stream and delta encodes then run length translates the data. It is up to

    The DELTA function only performs delta encoding of the input stream. 

    The RLE function performs run length encoding on the input stream. 

    Each function takes a function pointer (pWriteData) that a user can specify 
    for where bytes are written to. As bytes are calculated pWriteData will be called.
    By using this function pointer these functions can be adapted to run on embedded targets. 
    For example, this library was tested on a PC but runs on an embedded target where the pWriteByte
    is a function that writes a byte to UART.

*/
#include <stdint.h>

/* 
    Function transmits an input data stream that is delta run length encoded. 

    Arguments
        inputArray: Input data stream
        sizeInputArray: Size of the input data stream
        pWriteByte: Function for writing a byte
*/
void EncodeData_DRLE(uint16_t *inputArray, uint32_t sizeInputArray, void (*pWriteByte)(uint8_t))
{
    /* Declare variables used by this function */
    uint16_t currentValue = 0;
    uint16_t lastValue = 0;

    uint16_t currentDelta = 0;
    uint16_t lastDelta = 0;

    // Set the default value for count to 1. When storing this value, 1 will be subtracted, 
    // and if this defaults to 1, then no special logic is needed to make sure that the 
    // value wraps around if the count was zero.
    uint8_t count = 1;
    for(unsigned i = 0; i < sizeInputArray; i++)
    {

        /**********CALCULATE DELTA**********/
        /* Set current value */
        currentValue = inputArray[i];

        /* Use current value to calculate the current delta */
        currentDelta = lastValue - currentValue;

        /**********RUN LENGTH ENCODE**********/
        if(    (currentDelta == lastDelta)   // Current value in the array is the same as the previous
            && (count < 4)             // current value has not occurred 4 times yet.
          )
        {
            count = count + 1;
        }
        else
        {
            
            /*
                A small optimization is used with delta encoding. 
                    1 bit used to signal if number is 5 bits or 13 bits
                    2 bits for number of repetitions
                    13 bits for number
                Testing revealed that its more efficient to pack values into 16 bits. 
                THis observation that temperature measurements are able to fit with 13 bits, if 
                delta encoded. The values can actually fit into less bits since the delta between
                two points is usually fairy small. 

                Using this observation, if the value can fit within 5 bits, then the 5 bits 
                are used, otherwise the MSB will be set and then a 13 bit value will represent 
                the number.
            */
            if( lastDelta <= 0x1F)
            {
                /*
                    Output fits within 5 bits. Sent only a single encoded byte
                */
                uint8_t byteToSend = 0x00;
                /* MSB is set to 0 meaning value is 5 bits */
                byteToSend = byteToSend & 0xDF;
                /* 
                    Now Or in the count to the bits 6:5 

                    Count is stored with base of 1. Offset to 0. 
                    So now if in the byte 
                        00 = 1
                        01 = 2
                        10 = 3
                        11 = 4

                    Count is stored with the above mapping since there will NEVER be an instance where
                    a encoded byte is sent with a count of 0, as this would imply that the byte is not 
                    in the data stream. To maximize the count, represent the count subtracted by 1. This
                    allows count to represent 4 values instead of 3.
                */
                byteToSend = byteToSend | ((count-1) << 5 );
                /*
                    Low 5 bits represent the value. Or that in
                */
                byteToSend = byteToSend | (lastDelta & 0x1F);
                /* Use user specified write byte function to send the data out */
                pWriteByte( byteToSend);
                count = 1;
            }
            else
            {
                /*
                    Output does not fit within 5 bits. Sent only MSB to 1
                */
                uint8_t byteToSend = 0x00;
                /* MSB is set to 0 meaning value is 5 bits */
                byteToSend = byteToSend | 0x80;
                /* 
                    Now Or in the count to the bits 6:5 

                    Count is stored with base of 1. Offset to 0. 
                    So ow if in the byte 
                        00 = 1
                        01 = 2
                        10 = 3
                        11 = 4
                    
                    Count is stored with the above mapping since there will NEVER be an instance where
                    a encoded byte is sent with a count of 0, as this would imply that the byte is not 
                    in the data stream. To maximize the count, represent the count subtracted by 1. This
                    allows count to represent 4 values instead of 3.
                */
                byteToSend = byteToSend | ((count-1) << 5 );
                /*
                    Low 5 bits represent the 5 MSB of lastDelta
                */
                byteToSend = byteToSend | ((lastDelta>>8) & 0x1F);
                /* Use the user specified write byte function to send the data */
                pWriteByte( byteToSend);
                /*
                    Send the low 8 bits of lastDelta
                */
                byteToSend = ((lastDelta>>0) & 0xFF);
                /* Use the user specified write byte function to send the data */
                pWriteByte( byteToSend);
                /* Reset count back to 1 */
                count = 1;
            }
        }
        /* Update previous value */
        lastDelta = currentDelta;
        lastValue = currentValue;
    }
   
   /*
        The following if else dumps the remaining values. 

        If count is 2 or greater, then there was at least one value 
        read from the data stream that needs to be be written out. If count is
        1, which is the case only after writing data to file, then all data has been processed.
   */
    if(count >=2)
    {
        /* 
             See comments from above as this code is the same used above for checking if 
             on or two bytes should be written out.
        */
        if(lastDelta <= 0x1F)
        {
            uint8_t byteToSend = 0x00;
            byteToSend = byteToSend & 0xDF;
            byteToSend = byteToSend | ((count-1) << 5 );
            byteToSend = byteToSend | (lastDelta & 0x1F);
            pWriteByte( byteToSend);
            count = 1;
        }
        else
        {
            uint8_t byteToSend = 0x00;
            byteToSend = byteToSend | 0x80;
            byteToSend = byteToSend | ((count-1) << 5 );
            byteToSend = byteToSend | ((lastDelta>>8) & 0x1F);
            pWriteByte( byteToSend);
            byteToSend = ((lastDelta>>0) & 0xFF);
            pWriteByte( byteToSend);
            count = 1;
        }
    }
    
}

/* 
    Function transmits an input data stream that is delta encoded

    Arguments
        inputArray: Input data stream
        sizeInputArray: Size of the input data stream
        pWriteByte: Function for writing a byte
*/
void EncodeData_DELTA(uint16_t *inputArray, uint32_t sizeInputArray, void (*pWriteByte)(uint8_t))
{
    /*
        The first delta encoded integer is implicitly 0. 
        This first value is called the key value and is needed to encode the
        entire data stream.
    */
    uint16_t value = (0 - inputArray[0]);

    /*
        Data is transmitted MSB first

        Send the top 8 bits (most significant byte)
    */
    pWriteByte( (value>>8) & 0xFF);
    /* 
        Send the lower 8 bits (least significant byte)
    */
    pWriteByte( (value>>0) & 0xFF);
    for(unsigned i = 1; i < sizeInputArray; i++)
    {
        /* Delta encoding subtracts he current value from the previous value */
        value = inputArray[i-1] - inputArray[i];
        pWriteByte( (value>>8) & 0xFF);
        pWriteByte( (value>>0) & 0xFF);
    }
}

/* 
    Function transmits an input data stream that is run length encoded

    Arguments
        inputArray: Input data stream
        sizeInputArray: Size of the input data stream
        pWriteByte: Function for writing a byte
*/
void EncodeData_RLE(uint16_t *inputArray, uint32_t sizeInputArray, void (*pWriteByte)(uint8_t))
{
    /*
        Default current value to the first element in the first value in he input array.
    */
    uint16_t current = 0;
    uint16_t prev = inputArray[0];
    /* 
        Count is set to 1 since when setting currentSelectedValue 
        at least one element has the specified value.
    */
    uint8_t count = 1;

    for(unsigned i = 1; i < sizeInputArray; i++)
    {
        current = inputArray[i];
        if(    (current == prev)   // Current value in the array is the same as the previous
            && (count < 4)             // current value has not occurred 4 times yet.
          )
        {
            count = count + 1;
        }
        else
        {
            /* Output the count along with the value. */
            pWriteByte(count);
            pWriteByte( (prev>>8) & 0xFF);
            pWriteByte( (prev>>0) & 0xFF);
            count = 1;
        }
        /* Update previous value */
        prev = current;
    }
    /* Output any remaining value*/
    pWriteByte(count);
    pWriteByte( (prev>>8) & 0xFF);
    pWriteByte( (prev>>0) & 0xFF);
}
