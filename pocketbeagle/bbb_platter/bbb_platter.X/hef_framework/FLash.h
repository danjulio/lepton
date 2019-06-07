/*
 * Flash.h
 *
 * Updated on November 4, 2014
 */
//#include <xc.h>
#if defined( __16F1501)
//1K
    #define FLASH_ROWSIZE 16            // size of a row
    #define HEFLASH_START   0x0380      // first address in HE FLash memory
    #define HEFLASH_END     0x03FF      // last address in HE Flash memory
#elif defined( __16F1503) || defined( __16F1507) || defined( __16F1512) || \
      defined( __16F1703) || defined( __16F1707)
//2K
    #define FLASH_ROWSIZE 16            // size of a row
    #define HEFLASH_START   0x0780      // first address in HE FLash memory
    #define HEFLASH_END     0x07FF      // last address in HE Flash memory
#elif  defined( __16F1508) || defined( __16F1513) || \
       defined( __16F1704) || defined( __16F1708) || defined( __16F1713)
//4K
    #define FLASH_ROWSIZE 32            // size of a row
    #define HEFLASH_START   0x0F80      // first address in HE FLash memory
    #define HEFLASH_END     0x0FFF      // last address in HE Flash memory
#elif defined( __16F1509) || defined( __16F1526) || \
      defined( __16F1454) || defined( __16F1455) || defined( __16F1459) || \
      defined( __16F1705) || defined( __16F1709) || \
      defined( __16F1716) || defined( __16F1717)
//8K
    #define FLASH_ROWSIZE 32            // size of a row
    #define HEFLASH_START   0x1F80      // first address in HE FLash memory
    #define HEFLASH_END     0x1FFF      // last address in HE Flash memory
#elif defined( __16F1518_H) || defined( __16F1519_H) || defined( __16F1527) || \
      defined( __16F1718) || defined( __16F1719)
//16K
    #define FLASH_ROWSIZE 32            // size of a row
    #define HEFLASH_START   0x3F80      // first address in HE FLash memory
    #define HEFLASH_END     0x3FFF      // last address in HE Flash memory
#endif

#define FLASH_ROWMASK     FLASH_ROWSIZE-1   

/******************************************************************************
 * Generic Flash functions
 */

 /**
 * Read a word from program Flash memory
 *
 * @param address   source address (absolute FLASH memory address)
 * @return          word retrieved from FLASH memory
 */
unsigned FLASH_read( unsigned address);


/**
 * Read a word from configuration Flash memory
 *
 * @param address   source address (absolute FLASH memory address)
 * @return          word retrieved from FLASH memory
 */
unsigned FLASH_readConfig( unsigned address);

/**
 * Read a block of words from program Flash memory
 *
 * @param buffer    destination buffer (must be sufficiently large)
 * @param address   source address (absolute FLASH memory address)
 * @param count     number of words to be retrieved
 */
void    FLASH_readBlock( unsigned* buffer, unsigned address, char count);


/**
 * Write a word of data to Flash memory (latches)
 *  an actual write is performed only if LWLO = 0, data is latched if LWLO = 1
 *
 * @param address   destination address (absolute flash memory)
 * @param data      word of data to be written (latched)
 * @param latch     1 = latch, 0 = write
 */
void    FLASH_write( unsigned address, unsigned data, char latch);


/**
 * Erase a row of Flash memory
 *
 * @param address   absolute address in Flash contained in selected row
 */
void    FLASH_erase( unsigned address);


