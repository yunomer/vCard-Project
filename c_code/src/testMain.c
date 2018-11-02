#include <stdio.h>

#include "LinkedListAPI.h"
#include "VCardParser.h"
#include "VCardHelper.h"

int main (int argc, char ** argv) {

    char fileName[255];
    strcpy (fileName, argv[1]);

    Card * card = NULL;
    
    // *************************
    // READING IN OFFICIAL CARD!
    // *************************
    VCardErrorCode errorCode1 = createCard(fileName, &card);
    char * printErrorCode1 = printError(errorCode1);
    printf("**********************\n");
    printf("READING:\t%s", printErrorCode1);
    printf("**********************\n");
    free(printErrorCode1);

     // *************************
    // WRITING IN OFFICIAL CARD!
    // *************************
    const char writeCardName[10] = "temp.vcf";
    
    VCardErrorCode errorCode2 = writeCard(writeCardName, card);
    char * printErrorCode2 = printError(errorCode2);
    printf("**********************\n");
    printf("WRITING:\t%s", printErrorCode2);
    printf("**********************\n");
    free(printErrorCode2);
    deleteCard(card);
    card = NULL;

    // *************************
    // READING IN NEW CARD!
    // *************************
    VCardErrorCode errorCode3 = createCard((char*)writeCardName, &card);
    char * printErrorCode3 = printError(errorCode3);
    printf("**********************\n");
    printf("RE-READING:\t%s", printErrorCode3);
    printf("**********************\n");
    free(printErrorCode3);

    deleteCard(card);
    return 0;
}