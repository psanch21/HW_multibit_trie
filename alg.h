#include <stdint.h>


#ifndef ALG_H_
#define ALG_H_



/*---------------------------------------------------------------------------*/
/* Constant definitions                                                     */
/*---------------------------------------------------------------------------*/

#define TBL24_SIZE 16777216
#define TBLLong_SIZE 256





/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

uint32_t get24Index(uint32_t prefix, int prefixLength);
uint32_t getLast8Prefix(uint32_t prefix);
uint32_t getLongIndex(int16_t entry, uint32_t prefix);

int createTBLLong();
int initializeTBL24();
int insert(uint32_t prefix, int prefixLength, int outInterface);
int fillTBL24(uint32_t prefix, int prefixLength, int outInterface);
int fillTBLLong(uint32_t prefix, int prefixLength, int outInterface);
void freeTables();
int checkTBLLInEntry(uint16_t entry);

int lookup(uint32_t IPAddress, int * outInterface, int* numberOfHashtables);
#endif /* ALG_H_ */
