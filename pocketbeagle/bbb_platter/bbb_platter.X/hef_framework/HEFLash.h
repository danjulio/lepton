/*
 * HEFlash.h
 *
 */
#include "FLash.h"

#define HEFLASH_MAXROWS     (HEFLASH_END-HEFLASH_START+1)/FLASH_ROWSIZE

/******************************************************************************
 * High Endurance Flash functions
 */

/**
 * Write a block of data to High Endurance Flash
 *      the entire block must fit within a row 
 *
 * @param radd      HE Flash block number ( 0 - MAXROWS-1)
 * @param buffer    source buffer
 * @param count     number of bytes to write to block (< ROWSIZE)
 * @return          0 if successful, -1 if parameter error, 1 if write error
 */
char    HEFLASH_writeBlock( char radd, char* buffer, char count);


/**
 * Read a block of data from HE FLASH memory
 *
 * @param buffer    destination buffer (must be sufficiently large)
 * @param radd      source block of HE FLASH memory ( 0 - MAXROWS-1)
 * @param count     number of bytes to be retrieved ( < ROWSZE)
 * @return          0 if successful, -1 if parameter error
 */
char    HEFLASH_readBlock( char* buffer, char radd, char count);


/**
 * Read a byte of data from HE FLASH memory
 *
 * @param radd      source block of HE FLASH memory ( 0 - MAXROWS-1)
 * @param offset    offset within the HE block ( 0 - ROWSIZE-1)
 * @return          byte of data retrieved
 */
char    HEFLASH_readByte( char radd, char offset);

