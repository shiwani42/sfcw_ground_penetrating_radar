#include "arm_math.h"
// These functions functions are originally implemented in ARM CM assembly

/*
* @brief  In-place bit reversal function.
* @param[in, out] *pSrc        points to the in-place buffer of unknown 32-bit data type.
* @param[in]      bitRevLen    bit reversal table length
* @param[in]      *pBitRevTab  points to bit reversal table.
* @return none.
*/
void arm_bitreversal_16(uint16_t *pSrc, uint16_t bitRevLen, const uint16_t *pBitRevTable) {
    // C-implementation retrieved from https://community.arm.com/thread/10018
    uint32_t v0, v1, i;
    uint32_t *p0, *p1;

    for (i = (bitRevLen + 1u) >> 1; i > 0; i--) {
        p0 = (uint32_t *) ((uint8_t *) pSrc + (pBitRevTable[0] >> 1));
        p1 = (uint32_t *) ((uint8_t *) pSrc + (pBitRevTable[1] >> 1));
        v0 = *p0;
        v1 = *p1;
        *p0 = v1;
        *p1 = v0;

        pBitRevTable += 2;
    }
}


void arm_bitreversal_32(uint32_t *pSrc, const uint16_t bitRevLen, const uint16_t *pBitRevTable) {
    // C-implementation retrieved from https://community.arm.com/thread/9182
    uint32_t r7, r6, r5, r4, r3;
    if (bitRevLen <= 0)
        return;

    r3 = ((bitRevLen + 1u) >> 2);

    while (r3 > 0) {
        r7 = *(uint32_t *) ((uint8_t *) pSrc + pBitRevTable[3]);
        r6 = *(uint32_t *) ((uint8_t *) pSrc + pBitRevTable[2]);
        r5 = *(uint32_t *) ((uint8_t *) pSrc + pBitRevTable[1]);
        r4 = *(uint32_t *) ((uint8_t *) pSrc + pBitRevTable[0]);
        *(uint32_t *) ((uint8_t *) pSrc + pBitRevTable[3]) = r6;
        *(uint32_t *) ((uint8_t *) pSrc + pBitRevTable[2]) = r7;
        *(uint32_t *) ((uint8_t *) pSrc + pBitRevTable[1]) = r4;
        *(uint32_t *) ((uint8_t *) pSrc + pBitRevTable[0]) = r5;
        r7 = *(uint32_t *) ((uint8_t *) pSrc + pBitRevTable[3] + 4);
        r6 = *(uint32_t *) ((uint8_t *) pSrc + pBitRevTable[2] + 4);
        r5 = *(uint32_t *) ((uint8_t *) pSrc + pBitRevTable[1] + 4);
        r4 = *(uint32_t *) ((uint8_t *) pSrc + pBitRevTable[0] + 4);
        *(uint32_t *) ((uint8_t *) pSrc + pBitRevTable[3] + 4) = r6;
        *(uint32_t *) ((uint8_t *) pSrc + pBitRevTable[2] + 4) = r7;
        *(uint32_t *) ((uint8_t *) pSrc + pBitRevTable[1] + 4) = r4;
        *(uint32_t *) ((uint8_t *) pSrc + pBitRevTable[0] + 4) = r5;
        pBitRevTable += 4;
        r3--;
    }
}
