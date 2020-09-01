#include "decoding.h"

/* Decode a delta run length encoded data stream */
void DecodeData_DRLE(  uint8_t *inputArray , uint32_t sizeInputArray , void (*pWriteByte)(uint16_t))
{
    /* Declare the variables used through this function */
    uint8_t count        = 0;
    uint8_t encodedData  = 0;
    uint16_t delta       = 0;
    uint16_t value       = 0; // Seed value. Implicitly set to 0

    /* Iterate over the input stream */
    for(uint32_t i = 1; i < sizeInputArray; i++)
    {
        /* Read a value from the input array */
        encodedData = inputArray[i];
        /* If MSB is set, then value needs next byte to decode. */
        if(encodedData & 0x80)
        {
            /* Bits 6:5 contain the number of times the value should be replicated, offset by 1 */
            count = ((encodedData >> 5) & 0x3)+1;
            /* Delta value is encoded in 4:0 */
            uint16_t tmp = (((encodedData & 0x1F)<<8) | inputArray[i+1]);
            /* 
                Used an extra value from input array to calculate delta. Increment i to account for 
                the extra value read.
            */
            i = i + 1;
            /* Check MSB of delta to see if it needs to be sign extended. */
            delta = (tmp & 0x1000 ) ? ((tmp & 0x3FFF) | 0xE000) : (tmp & 0x3FFF);
            /* Repeat the value the specified number of times */
            for(uint32_t j = 0; j < count; j++)
            {
                 /* Calculate the value using the previous value and the current delta value */
                value = value - delta;

                /* Values are a total of 14 bits. */
                pWriteByte(value & 0x1FFF);
            }
        }
        /* All data encoded in 8 bits */
        else
        {
            /* Bits 6:5 contain the number of times the value should be replicated, offset by 1 */
            count = ((encodedData >> 5) & 0x3)+1;
            /* Delta value is encoded in 4:0 */
            /* 
                Using unsigned arithmetic. Do not need worry about sign extension since the value
                here is 5 bits.
             */
            delta = (encodedData & 0x1F);
            /* Repeat the value the specified number of times */
            for(uint32_t j = 0; j < count; j++)
            {
                /* Calculate the value using the previous value and the current delta value */
                value = value - delta;
                /* Send the decoded to the user specified write byte function */
                pWriteByte(value & 0x1FFF);
            }
        }
    }
}

/* Converts a delta encoded stream into its original stream */
void DecodeData_DELTA( uint8_t *inputArray , uint32_t sizeInputArray , void (*pWriteByte)(uint16_t))
{
    /* Delta encoding, uses pairs. */
    int16_t delta  = 0; 
    int16_t value  = 0; // Seed value. Implicitly set to 0
    /* Iterate over the entire input data stream */
    for(uint32_t i = 0; i < sizeInputArray; i = i + 2)
    {
        /* 
            Delta is stored as a 16 bit number. Take two elements from input stream and combine them
            into a single 16 bit value.
        */
        delta = (inputArray[i]<<8) | inputArray[i+1];
        /* use the previous value and the current delta to calculate the new value */
        value = value - delta;
        /* Send the byte out using the user specified write byte function */
        pWriteByte(value);
    }
}

/* Decode a run length encoded data stream */
void DecodeData_RLE(   uint8_t *inputArray , uint32_t sizeInputArray , void (*pWriteByte)(uint16_t))
{
    /* Count represents the repetitions of a number. Min = 1, and max = 255 */
    uint8_t count  = 0; 
    /* Values are stored as 16 bits. */
    int16_t value  = 0; // Seed value. Implicitly set to 0
    /* Iterate over entire input data stream */
    for(uint32_t i = 0; i < sizeInputArray; i = i + 3)
    {
        /* Read the count from the input array. Count value always precedes a vale. */
        count = inputArray[i];
        /* Next two bytes store the value. Combine both bytes to create the value */
        value = (inputArray[i+1]<<8) | inputArray[i+2];
        /* Run "count" number of times. This replicates the value */
        for(uint32_t j = 0; j < count; j++)
        {
            /* Send the value using the user specified write byte function */
            pWriteByte(value);
        }
    }
}