#ifndef DECODING_H_
#define DECODING_H_

#include <stdint.h>


void DecodeData_DRLE(  uint8_t *inputArray , uint32_t sizeInputArray , void (*pWriteByte)(uint16_t));
void DecodeData_DELTA( uint8_t *inputArray , uint32_t sizeInputArray , void (*pWriteByte)(uint16_t));
void DecodeData_RLE(   uint8_t *inputArray , uint32_t sizeInputArray , void (*pWriteByte)(uint16_t));

#endif //DECODING_H_