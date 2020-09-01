#ifndef ENCODING_H_
#define ENCODING_H_



#include <stdint.h>


void EncodeData_DRLE( uint16_t *inputArray, uint32_t sizeInputArray, void (*pWriteByte)(uint8_t));
void EncodeData_DELTA(uint16_t *inputArray, uint32_t sizeInputArray, void (*pWriteByte)(uint8_t));
void EncodeData_RLE(  uint16_t *inputArray, uint32_t sizeInputArray, void (*pWriteByte)(uint8_t));

#endif //ENCODING_H_