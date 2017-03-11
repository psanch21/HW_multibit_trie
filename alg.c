
#include <math.h>
#include "alg.h"
#include "aux.h"
#include "io.h"


// http://stackoverflow.com/questions/10079056/what-does-the-in-int-30-mean OJO


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
uint16_t TBL24[TBL24_SIZE];
uint16_t* TBLLong;
uint16_t numOfTBLL;
/*---------------------------------------------------------------------------*/
/* Definition of functions                                                   */
/*---------------------------------------------------------------------------*/


int initializeTBL24(){
  uint32_t i;
  for(i = 0; i<TBL24_SIZE;i++){
    TBL24[i] = 0;
  }
  return OK;
}

/*---------------------------------------------------------------------------*/
/* Helper functions                                                  */
/*---------------------------------------------------------------------------*/

int createTBLLong(){
  TBLLong = (uint16_t*)realloc(TBLLong,(numOfTBLL+1)*sizeof(int16_t)*256);

  if(TBLLong==NULL){
    free(TBLLong);
    return ALLOC_FAIL;
  }
  numOfTBLL++;
  return OK;

}

int checkTBLLInEntry(int16_t entry){
  if(entry >=32768){//2 to the power of 15
    return 1;//There is TBL
  }else{
    return 0;
  }
}

uint32_t get24Index(uint32_t prefix){
  uint32_t tmp = prefix & 0xffffff00;
  tmp = tmp>>8;
  return tmp;
}

uint32_t getLongIndex(int16_t entry, uint32_t prefix){
  uint16_t low =(uint16_t) prefix & 0x000000ff;
  return (entry & 0x7fff) + low;
}
uint32_t getLast8Prefix(uint32_t prefix){
  return prefix & 0x000000ff;
}
/*---------------------------------------------------------------------------*/
/* Functions to fill tables                                                  */
/*---------------------------------------------------------------------------*/


int fillTBL24(uint32_t prefix, int prefixLength, int outInterface){
  uint32_t index24 = get24Index(prefix);
  uint32_t i;
  uint32_t count = pow(2,24-prefixLength);
  for(i=index24; i < (index24+count);i++){
    TBL24[i] = (uint16_t)outInterface;
  }
  return OK;
}

int fillTBLLong(uint32_t prefix, int prefixLength, int outInterface){
  uint32_t index24 = get24Index(prefix);
  if(checkTBLLInEntry(TBL24[index24])){
    uint32_t start = (TBL24[index24] & 0x7fff)*256;
    uint32_t i;
    uint32_t j = 0;
    uint32_t lowbyte = getLast8Prefix(prefix);
    uint32_t count = pow(2,32 - prefixLength );
    for(i = start; i<start + 256;i++){
      if(j>=lowbyte && j< lowbyte +count){
       TBLLong[i] =(uint16_t) outInterface;
      }
      j++;
    }

  }else{ //Create TBLLong 
    int16_t tmpEntry = TBL24[index24];
    uint16_t indexLong = numOfTBLL;
    TBL24[index24] =indexLong | 0x8000;

    uint32_t lowbyte = getLast8Prefix(prefix);

    createTBLLong(prefix);//This method increases numOfTBLL

    int start = indexLong*256;
    int i;
    int j = 0;
    int count = pow(2,32 - prefixLength );
    for(i = start; i<start + 256;i++){
      if(j<lowbyte || j> lowbyte +count){
        TBLLong[i] = tmpEntry;
      }else{
       TBLLong[i] = (uint16_t)outInterface;
      }
      j++;
    }
  }
  

  return OK;
}

int insert(uint32_t prefix, int prefixLength, int outInterface){


  if(prefixLength<=24){ //Insert outInterface in TBL24
    return fillTBL24(prefix,prefixLength,outInterface);
  }else{//Insert information to look in TBLLong
    return fillTBLLong(prefix,prefixLength,outInterface);
  }

  return OK;
}

void freeTables(){
  free(TBLLong);
}



int lookup(uint32_t IPAddress, int * outInterface, int* numberOfHashtables){
  uint32_t index24 = get24Index(IPAddress);
  uint16_t entry24 = TBL24[index24];
  if(entry24>=32768){
    uint32_t lowbyte = getLast8Prefix(IPAddress);
     uint32_t longIndex = ((TBL24[index24] & 0x7fff)*256) + lowbyte;
     *outInterface = TBLLong[longIndex];
    *numberOfHashtables = 2;
  }else{
    *outInterface =entry24;
    *numberOfHashtables = 1;
  }
  return OK;
}


/*---------------------------------------------------------------------------*/
/* Main function                                                       */
/*---------------------------------------------------------------------------*/



int main(int argc, char  *argv[]) {

  if(argc !=3){
    printf("Wrong parameters: routingTableName inputFileName\n");
    return 0;
  }

  int result = OK; //Variable to handle errors



  //Initializacion

  if((result = initializeIO(argv[1],argv[2])) != OK){
    printIOExplanationError(result);
  }

  initializeTBL24();
  TBLLong = NULL;
  numOfTBLL=0;



  // Insert
  uint32_t prefix;
  int prefixLength;
  int outInterface;

  while((result =readFIBLine(&prefix, &prefixLength, &outInterface)) == OK){
    result = insert(prefix, prefixLength,outInterface);
  }

  if(result != REACHED_EOF){
    printIOExplanationError(result);
  }
  
  
  //Look up
  uint32_t IPAddress =0;
  int outIfc = 0;
  int numberOfHashtables = 0;
  struct timeval initialTime;
  struct timeval finalTime;
  double searchingTime;

  int processedPackets = 0;
  double totalTableAccesses = 0;
  double totalPacketProcessingTime = 0;

  while((result =readInputPacketFileLine(&IPAddress)) == OK){
    gettimeofday(&initialTime, NULL);
    result = lookup(IPAddress,&outIfc,&numberOfHashtables);
    gettimeofday(&finalTime, NULL);
    printOutputLine(IPAddress, outIfc,&initialTime,&finalTime,&searchingTime,numberOfHashtables);

    processedPackets++;
    totalTableAccesses+=numberOfHashtables;
    totalPacketProcessingTime+= searchingTime;
  }

  if(result != REACHED_EOF){
    printIOExplanationError(result);
  }

/*
  int in = 5;

  printf("TBLLong[%u] = %u\n",in,TBLLong[in]);

  */
  
  //Closing
  double averageTableAccesses = totalTableAccesses/processedPackets;
  double averagePacketProcessingTime = totalPacketProcessingTime/processedPackets;
  
  printSummary(processedPackets,averageTableAccesses,averagePacketProcessingTime);
  
  freeIO();
  freeTables();


  return 0;
}
