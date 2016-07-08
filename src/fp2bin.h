#pragma once
/***********************************************************/
/* fp2bin.h: Convert IEEE double to binary string          */
/*                                                         */
/* Rick Regan, http://www.exploringbinary.com              */
/*                                                         */
/***********************************************************/
/* FP2BIN_STRING_MAX covers the longest binary string
   (2^-1074 plus "0." and string terminator) */
#define FP2BIN_STRING_MAX 1077

void fp2bin(double fp, char* binString);
