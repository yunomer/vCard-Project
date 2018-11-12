#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "VCardParser.h"
#include "VCardHelper.h"
#include "LinkedListAPI.h"


char* cEngine (char* fileName) {
    if (fileName == NULL) {
        return NULL;
    }

    Card * card = NULL;
    VCardErrorCode errorCode = createCard(fileName, &card);
    if (errorCode != OK) {
        char * code = printError(errorCode);
        return code;
    }
    char * cardToJsonString = cardToJson(card);
    return cardToJsonString;
}

bool validateFileExtention (char * fileName) {
    char * validExt1 = malloc(sizeof(char)*10);
    char * validExt2 = malloc(sizeof(char)*10);
    char fileExtention[6];

    // first clean the file name by removing the potential newline char
    int fileNameLen = strlen(fileName);
    if (fileName[fileNameLen - 1] == '\n') {
        fileName[fileNameLen - 1] = '\0';
    }

    // create a copy of the file name into a new variable
    strncpy(fileExtention, &fileName[fileNameLen - 5], 5);

    strcpy(validExt1, ".vcard");
    strcpy(validExt2, ".vcf");

    int result = strcmp (validExt1, fileExtention);
    if (result == 0) {
        free(validExt1);
        free(validExt2);
        return true;
    }
    else {
        result = strcmp (validExt2, fileExtention); 
        if (result != 0) {
            free(validExt1);
            free(validExt2);
            return true;
        }
    }

    free(validExt1);
    free(validExt2);
    return false;
}

VCardErrorCode preProcessFile (char * fileName, char *** fileArray) {
   // pointer to the file, will be set later. 
   FILE * ptr = NULL;

    // First check to see if the file char pointer is not NULL
    if (fileName == NULL) {
        return -1;
    }
    
   // Check to see if file extention is valid
   bool fileExtention = validateFileExtention(fileName);
   if (fileExtention == false) {
       return -1;
   }

    // first clean the file name by removing the potential newline char
    int fileNameLen = strlen(fileName);
    if (fileName[fileNameLen - 1] == '\n') {
        fileName[fileNameLen - 1] = '\0';
    }

   // Attempt to open the file
   ptr = fopen(fileName, "r");

   //If the file failed to open, return -1
   if (ptr == NULL) {
       return -1;
   }

    //********************************************************************
    // Start pre-processing the file
    // Remove the line folds and stuff...
    //********************************************************************

    int fileArraySize = 1;
    // Malloc the fileArray to hold the number of lines.
    *fileArray = malloc(sizeof(char*) * fileArraySize);

    // Create the loading array && holding array
    char loadingArray[100];
    char * storeString = NULL;

    // Keep track of line number
    int lineNumber = 0;

    // variable to record string Size
    int stringSize = 0;
    int tempSize = 0;

    // Get Line
    fgets(loadingArray, 100, ptr);
    char * holdingArray = NULL;

    while(!feof(ptr)) {
        // Get the size of the loading array so we can malloc the holding array to it
        stringSize = strlen(loadingArray);
        // Malloc the holding array to exactly fit the loading array size.
        holdingArray = malloc(sizeof(char) * stringSize + 1);
        // Copy line into holding array so next line can be copied.
        // This is to evaluate if next line is a folded line
        strncpy(holdingArray, loadingArray, stringSize + 1);
        //Now we need to clean the loadingArray
        for(int i = 0; i < 100; i++) {
            loadingArray[i] = '\0';
        }
        // Get next line
        fgets(loadingArray, 100, ptr);
        // This represents the structure needed for a folded line.
        if ((holdingArray[stringSize-2] == '\r') && (holdingArray[stringSize-1] == '\n') && ((loadingArray[0] == ' ') || (loadingArray[0] == '\t'))) {
            while((holdingArray[stringSize-2] == '\r') && (holdingArray[stringSize-1] == '\n') && ((loadingArray[0] == ' ') || (loadingArray[0] == '\t'))) {
                // Recording the initial Size of both arrays
                int holdingArraySize = strlen(holdingArray);
                int loadingArraySize = strlen(loadingArray);

                // Adding them, as this will be the new array size
                tempSize = holdingArraySize + loadingArraySize;

                // First remove the carrage return and newline chars from holding array
                holdingArray[holdingArraySize-2] = '\0';
                holdingArray[holdingArraySize-1] = '\0';

                // Now create new area to hold the lines
                char * tempCatString = malloc (sizeof(char) * tempSize + 1);

                // Copy holdingArray into tempCatString, then concat loading Array
                strncpy(tempCatString, holdingArray, holdingArraySize-1);

                // Create this temp pointer to point to loading array position one
                // This is to make sure we don't copy the space.
                char * tempMover = &(loadingArray[1]);

                strcat(tempCatString, tempMover);

                // Now give this pointer back to holding Array
                // to do this, make sure to free holding array and loading array
                // Set to NULL
                free(holdingArray);
                
                holdingArray = NULL;
                //Now we need to clean the loadingArray
                for(int i = 0; i < 100; i++) {
                    loadingArray[i] = '\0';
                }
                
                // Hand off tempCatString to HoldingArray pointer to make that the new Holding array.
                // The Carrage return and New Line should still exists here to remain in this while loop
                holdingArray = tempCatString;
                tempCatString = NULL;

                // Re-evaulate the stringSize for holdingArray since we changed it.
                stringSize = strlen(holdingArray);

                // Get next line
                fgets(loadingArray, 100, ptr);
            }
        }

         // This will remove the carrage return and new line character from the end of the holding array line
       /* if ((holdingArray[stringSize-2] == '\r') && (holdingArray[stringSize-1] == '\n')) {
            holdingArray[stringSize-2] = '\0';
            holdingArray[stringSize-1] = '\0';
        }*/

        // Record the String size
        stringSize = strlen(holdingArray);
        // Create new memory to store the final string and copy string from holding array to the new mem loc.
        storeString = malloc(sizeof(char) * stringSize + 1);
        strncpy(storeString, holdingArray, stringSize + 1);

        // Move the pointer from storeString to final location of fileArray.
        (*fileArray)[lineNumber] = storeString;

        fileArraySize++;

        (*fileArray) = realloc((*fileArray), (sizeof(char*) * fileArraySize));

        // Make sure the storeString does not contain the pointer anymore.
        storeString = NULL;

        // Also make sure that your Holding array is free and malloc'ed again.
        free(holdingArray);

        // Increment the line Number and repeat.
        lineNumber++;
    }

    free(storeString);
    fclose(ptr);
    return fileArraySize-1;
}

bool initialCheckFile (char *** fileArray, int arraySize) {

    int result = 0;

    if ((fileArray == NULL) || (arraySize <= 3)) {
        return false;
    }

    // Here is block to check if beginCheck exists.
    char * beginCheck = malloc(sizeof(char) * 20);
    strcpy(beginCheck, "BEGIN:VCARD\r\n");
    result = strcmp(beginCheck, (*fileArray)[0]);
    if (result != 0) {
        free(beginCheck);
        return false;
    }
    free(beginCheck);


    // Here is block to see if version exists
    char * versionCheck = malloc(sizeof(char) * 20);
    strcpy(versionCheck, "VERSION:4.0\r\n");
    result = strcmp(versionCheck, (*fileArray)[1]);
    if (result != 0) {
        free(versionCheck);
        return false;
    }
    free(versionCheck);

    // Here is block to see if Endcheck exists
    char * endCheck = malloc(sizeof(char) * 20);
    strcpy(endCheck, "END:VCARD\r\n");
    result = strcmp(endCheck, (*fileArray)[arraySize-1]);
    if (result != 0) {
        free(endCheck);
        return false;
    }
    free(endCheck);

    // Now we have to check if the FN tag exists!
    char * copyString = NULL;
    for(int i = 0; i < arraySize; i++) {
        char * string = (*fileArray)[i];
        int stringLen = strlen(string);
        copyString = malloc(sizeof(char) * stringLen + 1);
        strcpy(copyString, string);

        for(int j = 0; j < stringLen; j++) {
            copyString[j] = tolower(copyString[j]);
        }

        if ((copyString[0] == 'f') && (copyString[1] == 'n') && (copyString[2] == ':')) {
            free(copyString);
            copyString = NULL;
            return true;
        }

        free(copyString);  
        copyString = NULL; 
    }
    return false;
}

void deleteSavedFile ( char *** fileArray, int arraySize) {
    if (((*fileArray) == NULL) || arraySize <= 0) {
        return;
    }
    for(int i = 0; i < arraySize; i++) {
        if ((*fileArray)[i] != NULL) {
             free((*fileArray)[i]);
        }
    }
    free(*fileArray);
}

char * stringSlicer (char ** string, int start, int end) {
    char * returnString = malloc(sizeof(char) * (end-start) + 2);
    int startMoving = start;
    int endMoving = end;
    char * temp = returnString;
    char * mover = &(*string)[startMoving];

    while (startMoving < endMoving) {
        *temp++ = *mover++;
        startMoving++;
    }
    returnString[end-start] = '\0';
    return returnString;
}

VCardErrorCode fnPropertyParser ( char ** string, Card** obj) {
    Property * ptr = (*obj)->fn;
    
    int stringLen = strlen(*string);

    char colon = ':';
    char semiColon = ';';
    char equle = '=';
    char escape = '\\';
    char group = '.';

    int start = 0;
    int end = 0;

    bool propColon = false;
    bool propSemiColon = false;

    // Here we will find the Prop, and if the Prop ends with a colon or semi-colon. 
    for (int i = 0; i < stringLen; i++) {
        if ((start == 0) && (((*string)[i] == colon) || ((*string)[i] == semiColon))) {
            end = i;

            if ((*string)[i] == colon) {
                propColon = true;
            } else {
                propSemiColon = true;
            }

            char * returnString = stringSlicer(string, start, end);

            int propLen = strlen(returnString);
            
            if (propLen < 1) {
                free(returnString);
                printf("I was here!\n");
                return INV_PROP;
            }

            // Check to see if the Prop tag found is not a Param.
            for (int j = 0; j < strlen(returnString); j++) {
                if (returnString[j] == equle) {
                    free(returnString);
                    return INV_PROP;
                }
            }

            // Check to see if the Prop tag has a Group too
            for (int j = 0; j < strlen(returnString); j++) {
                if (returnString[j] == group) {
                    int dotLoc = j;
                    char * groupName = stringSlicer(string, start, dotLoc);
                    free(ptr->group);
                    ptr->group = groupName;
                    free(returnString);
                    returnString = NULL;
                    returnString = stringSlicer(string, dotLoc+1, end);
                }
            }

            // This means that FN does not have a value, hence invalid.
            if (end == stringLen-1) {
                free(returnString);
                return INV_PROP;
            }

            free(ptr->name);
            ptr->name = returnString;

            start = end+1;
            break;
        }
    }


    bool firstParamFound = false;
    bool restIsValues = false;

    // Now we can start processing the next pieces of text.
    for (int i = start; i <= stringLen; i++) {

        if ((propColon == true) && (propSemiColon == false)) {
            if (((*string)[i] == semiColon) || (i == stringLen)) {
                if ((*string)[i-1] != escape) {
                    end = i;

                    char * returnString = stringSlicer(string, start, end);

                    if (strlen(returnString) < 1) {
                        free(returnString);
                        return INV_PROP;
                    }

                    // Check to see if there's a Param in the string, if there is, make sure it's complete
                    for (int j = 0; j < strlen(returnString); j++) {
                        if (returnString[j] == equle) {
                            if ((j > 0) && (j < strlen(returnString)-1)) {
                                break;
                            }
                            else {
                                free(returnString);
                                return INV_PROP;
                            }
                        }
                    }

                    insertBack(ptr->values, (void*)returnString);
                    returnString = NULL;

                    start = i+1;
                }
            }
        }

        if ((propColon == false) && (propSemiColon == true)) {
            if (((*string)[i] == semiColon) || ((*string)[i] == colon) ||(i == stringLen)) {
                end = i;

                if (i == stringLen-1) {
                    end = stringLen;
                }

                if (firstParamFound == true) {

                    if (restIsValues == false) {

                        if ((*string)[i] == colon) {
                            restIsValues = true;
                        }
                        char * returnString = stringSlicer(string, start, end);
                        // Check to make sure this is a Param and is a valid one... if it's not, then return Error
                        bool foundEqule = false;
                        for (int j = 0; j < strlen(returnString); j++) {
                            if (returnString[j] == equle) {
                                if ((j >= 1) && (j < strlen(returnString))) {
                                    foundEqule = true;
                                }
                            }
                        }
                        if (foundEqule == false) {
                            free(returnString);
                            return INV_PROP;
                        }
                        Parameter * ptrToParam = NULL;
                        VCardErrorCode temp = paramProcess(&returnString, &ptrToParam);
                        if (temp != OK) {
                            free(returnString);
                            return temp;
                        }
                        insertBack(ptr->parameters, (void*)ptrToParam);
                        firstParamFound = true;
                        if (returnString != NULL) {
                            free(returnString);
                            returnString = NULL;
                        }
                        start = i+1;
                    }

                    if (restIsValues == true) {
                        if (((*string)[i] == semiColon) || (i == stringLen)) {
                            if ((*string)[i-1] != escape) {
                                end = i;

                                char * returnString = stringSlicer(string, start, end); 
                                // Check to see if there's a Param in the string, if there is, make sure it's complete
                                for (int j = 0; j < strlen(returnString); j++) {
                                    if (returnString[j] == equle) {
                                        if ((j > 0) && (j < strlen(returnString)-1)) {
                                            break;
                                        }
                                        else {
                                            free(returnString);
                                            return INV_PROP;
                                        }
                                    }
                                }

                                insertBack(ptr->values, (void*)returnString);
                                returnString = NULL;

                                start = i+1;
                            }
                        }
                    }

                }

                if (firstParamFound == false) {
                    if ((*string)[i] == colon) {
                        restIsValues = true;
                    }
                    char * returnString = stringSlicer(string, start, end);
                    // Check to make sure this is a Param and is a valid one... if it's not, then return Error
                    bool foundEqule = false;
                    for (int j = 0; j < strlen(returnString); j++) {
                        if (returnString[j] == equle) {
                            if ((j >= 1) && (j < strlen(returnString))) {
                                foundEqule = true;
                            }
                        }
                    }
                    if (foundEqule == false) {
                        free(returnString);
                        return INV_PROP;
                    }

                    Parameter * ptrToParam = NULL;

                    VCardErrorCode temp = paramProcess(&returnString, &ptrToParam);

                    if (temp != OK) {
                        free(returnString);
                        return temp;
                    }

                    insertBack(ptr->parameters, (void*)ptrToParam);

                    if (returnString != NULL) {
                        free(returnString);
                        returnString = NULL;
                    }
                    firstParamFound = true;
                    start = i+1;
                }
            }
        } 
    }
    return OK;
}

VCardErrorCode dateTimeParser ( char ** string, DateTime ** obj) {
    DateTime * ptr = malloc(sizeof(DateTime) + sizeof(char) * 200);
    (*obj) = ptr;

    char colon = ':';
    char semiColon = ';';
    char equle = '=';

    int stringLen = strlen(*string);

    bool paramFound = false;
    
    int start = 0;
    int end = 0;

    //In this loop, find the Tag "BDAY" or "ANNIVERSARY" && set the Start location.
    for (int i = 0; i < stringLen; i++) {
        if ((start == 0) && (((*string)[i] == colon) || ((*string)[i] == semiColon))) {
            end = i;

            if ((*string)[i] == semiColon) {
                paramFound = true;
            }

            char * returnString = stringSlicer(string, start, end);

            // Check to see if the Birthday/Anni tag found is not a Param.
            for (int j = 0; j < strlen(returnString); j++) {
                if (returnString[j] == equle) {
                    free(returnString);
                    return INV_PROP;
                }
            }
            free(returnString);
            start = i + 1;
            break;
        }
    }

    bool localUTC = false;
    bool localText = false;

    // TODO ========> Don't forget to free these suckers
    char * dateString = NULL;
    char * timeString = NULL;
    char * timeSubString = NULL;

    char * returnString = NULL;

    int stop = 0;
    // Now we know that the prop is okay, we check to see if the next string is a Parameter.
    // If next string is parameter, then we make UTC false, and isText true.
    for (int i = start; i < stringLen; i++) {
        if ((((*string)[i] == colon) || ((*string)[i] == semiColon) || (i == stringLen-1)) && stop == 0) {
            end = i;
            if (i == stringLen-1) {
                end = stringLen;
            }

            returnString = stringSlicer(string, start, end);

            if (paramFound == true) {
                for (int j = 0; j < strlen(returnString); j++) {
                    if ((returnString[j] == equle) && (returnString[j+1] == 't')) {
                        localText = true;
                        start = i + 1;
                        stop = 1;
                    }
                }
            }
            if (localText == false) {
                start = i + 1;
                stop = 1;
            }
        }
    }

    if (localText == true) {
        free(returnString);
        returnString = NULL;

        returnString = stringSlicer(string, start, stringLen);

        ptr->isText = true;
        ptr->UTC = false;
        strcpy(ptr->text, returnString);

        free(returnString);
        returnString = NULL;

        strcpy(ptr->date, "");
        strcpy(ptr->time, "");
        if (returnString != NULL) {
            free (returnString);
            returnString = NULL;
        }
        if (dateString != NULL) {
            free (dateString);
            dateString = NULL;
        }
        if (timeString != NULL) {
            free (timeString);
            timeString = NULL;
        }
        return OK;
    }

    if (localText == false) {
        start = 0;
        stringLen = strlen(returnString);

        // First we have to check to see if the string contains a colon or semicolon.
        for (int i = start; i < stringLen; i++) {
            if ((start == 0) && ((returnString[i] == colon) || (returnString[i] == semiColon))) {
                free(returnString);
                return INV_PROP;
            }
        }

        char empty[1] = "";

        for (int i = start; i < stringLen; i++) {
            if (returnString[i] == 'T') {
                end = i;
                dateString = stringSlicer(&returnString, start, end);
                timeString = stringSlicer(&returnString, end + 1, stringLen);

                if(timeString[strlen(timeString)-1] == 'Z') {
                    localUTC = true;
                    free(timeString);
                    timeString = stringSlicer(&returnString, end + 1, stringLen-1);
                }
                
                free(returnString);

                returnString = NULL;
                int dateStringLen = strlen(dateString);
                // This is so that if the string only has time.
                if (dateStringLen <= 1) {
                    free(dateString);
                    dateString = NULL;
                }
                break;
            }
        }

        // This means that time does not exist... Only Thing there is a date.
        if ((returnString != NULL) && (dateString == NULL) && (timeString == NULL)) {
            ptr->UTC = localUTC;
            ptr->isText = localText;
            strcpy(ptr->date, returnString);
            strcpy(ptr->time, empty);
            strcpy(ptr->text, empty);
        } 

        // This means that only time exists...
        if ((dateString == NULL) && (timeString != NULL)) {

            for (int j = 0; j < strlen(timeString); j++) {
                if ((timeString[j] == '-') && (isdigit(timeString[j-1]))) {
                    timeSubString = stringSlicer(&timeString, 0, j);
                    free(timeString);
                    timeString = NULL;
                }
            }

            ptr->UTC = localUTC;
            ptr->isText = false;
            strcpy(ptr->text, empty);
            strcpy(ptr->date, empty);
            if(timeSubString) {
                strcpy(ptr->time, timeSubString);
            } else {
                strcpy(ptr->time, timeString);
            }   
        }

        // This means that both date AND time exist...
        if ((dateString != NULL) && (timeString != NULL) && (returnString == NULL)) {
            // Deal with dateString first...
            stringLen = strlen(dateString);

            char empty[1] = "";

            for (int j = 0; j < strlen(timeString); j++) {
                if ((timeString[j] == '-') && (isdigit(timeString[j-1]))) {
                    timeSubString = stringSlicer(&timeString, 0, j);
                    free(timeString);
                    timeString = NULL;
                    break;
                }
            }

            ptr->UTC = localUTC;
            ptr->isText = localText;

            strcpy(ptr->text, empty);
            strcpy(ptr->date, dateString);
            if(timeSubString != NULL) {
                strcpy(ptr->time, timeSubString);
                free(timeSubString);
                timeSubString = NULL;
            } 
            if (timeSubString == NULL) {
                strcpy(ptr->time, timeString);
                free(timeString);
                timeString = NULL;
            }
        }
    }

    if (returnString != NULL) {
        free (returnString);
        returnString = NULL;
    }
    if (dateString != NULL) {
        free (dateString);
        dateString = NULL;
    }
    if (timeString != NULL) {
        free (timeString);
        timeString = NULL;
    }

    return OK;
}

VCardErrorCode optionalPropertyParser ( char ** string, Card** obj) {
    Property * ptr = malloc(sizeof(Property));
    ptr->parameters = initializeList(&printParameter, &deleteParameter, &compareParameters);
    ptr->values = initializeList(&printValue, &deleteValue, &compareValues);
    ptr->name = malloc(sizeof(char));
    // ===> THIS WAS MALLOC'ed TO FULFILL THE REQUIRMENT OF NOT BEING NULL
    ptr->group = malloc(sizeof(char) * 1);
    strcpy(ptr->group, "");

    int stringLen = strlen(*string);

    char colon = ':';
    char semiColon = ';';
    char equle = '=';
    char escape = '\\';
    char group = '.';

    int start = 0;
    int end = 0;

    bool propColon = false;
    bool propSemiColon = false;

    // Here we will find the Prop, and if the Prop ends with a colon or semi-colon. 
    for (int i = 0; i < stringLen; i++) {
        if ((start == 0) && (((*string)[i] == colon) || ((*string)[i] == semiColon))) {
            end = i;

            if ((*string)[i] == colon) {
                propColon = true;
            } else {
                propSemiColon = true;
            }

            char * returnString = stringSlicer(string, start, end);

            int propLen = strlen(returnString);
            if (propLen < 1) {
                free(returnString);
                deleteProperty(ptr);
                return INV_PROP;
            }

            // Check to see if the Prop tag found is not a Param.
            for (int j = 0; j < strlen(returnString); j++) {
                if (returnString[j] == equle) {
                    free(returnString);
                    return INV_PROP;
                }
            }

            // Check to see if the Prop tag has a Group too
            for (int j = 0; j < strlen(returnString); j++) {
                if (returnString[j] == group) {
                    int dotLoc = j;
                    char * groupName = stringSlicer(string, start, dotLoc);
                    free(ptr->group);
                    ptr->group = groupName;
                    free(returnString);
                    returnString = NULL;
                    returnString = stringSlicer(string, dotLoc+1, end);
                }
            }

            // This means that FN does not have a value, hence invalid.
            if (end == stringLen-1) {
                return INV_PROP;
            }

            free(ptr->name);
            ptr->name = returnString;

            start = end+1;
            break;
        }
    }


    bool firstParamFound = false;
    bool restIsValues = false;

    // Now we can start processing the next pieces of text.
    for (int i = start; i <= stringLen; i++) {

        if ((propColon == true) && (propSemiColon == false)) {
            if (((*string)[i] == semiColon) || (i == stringLen)) {
                if ((*string)[i-1] != escape) {
                    end = i;

                    char * returnString = stringSlicer(string, start, end); 

                    // Check to see if there's a Param in the string, if there is, make sure it's complete
                    for (int j = 0; j < strlen(returnString); j++) {
                        if (returnString[j] == equle) {
                            if ((j > 0) && (j < strlen(returnString)-1)) {
                                break;
                            }
                            else {
                                free(returnString);
                                return INV_PROP;
                            }
                        }
                    }

                    insertBack(ptr->values, (void*)returnString);
                    returnString = NULL;

                    start = i+1;
                }
            }
        }

        if ((propColon == false) && (propSemiColon == true)) {
            if (((*string)[i] == semiColon) || ((*string)[i] == colon) || (i == stringLen)) {
                end = i;

                if (i == stringLen-1) {
                    end = stringLen;
                }

                if (firstParamFound == true) {

                    if (restIsValues == false) {

                        if ((*string)[i] == colon) {
                            restIsValues = true;
                        }
                        char * returnString = stringSlicer(string, start, end);
                        // Check to make sure this is a Param and is a valid one... if it's not, then return Error
                        bool foundEqule = false;
                        for (int j = 0; j < strlen(returnString); j++) {
                            if (returnString[j] == equle) {
                                if ((j >= 1) && (j < strlen(returnString))) {
                                    foundEqule = true;
                                }
                            }
                        }
                        if (foundEqule == false) {
                            free(returnString);
                            deleteProperty(ptr);
                            return INV_PROP;
                        }
                        Parameter * ptrToParam = NULL;
                        VCardErrorCode temp = paramProcess(&returnString, &ptrToParam);
                        if (temp != OK) {
                            deleteProperty(ptr);
                            free(returnString);
                            return temp;
                        }
                        insertBack(ptr->parameters, (void*)ptrToParam);
                        firstParamFound = true;
                        if (returnString != NULL) {
                            free(returnString);
                            returnString = NULL;
                        }
                        start = i+1;
                    }

                    if (restIsValues == true) {
                        if (((*string)[i] == semiColon) || (i == stringLen)) {
                            if ((*string)[i-1] != escape) {
                                end = i;

                                char * returnString = stringSlicer(string, start, end); 
                                // Check to see if there's a Param in the string, if there is, make sure it's complete
                                for (int j = 0; j < strlen(returnString); j++) {
                                    if (returnString[j] == equle) {
                                        if ((j > 0) && (j < strlen(returnString)-1)) {
                                            break;
                                        }
                                        else {
                                            free(returnString);
                                            deleteProperty(ptr);
                                            return INV_PROP;
                                        }
                                    }
                                }

                                insertBack(ptr->values, (void*)returnString);
                                returnString = NULL;

                                start = i+1;
                            }
                        }
                    }

                }

                if (firstParamFound == false) {
                    if ((*string)[i] == colon) {
                        restIsValues = true;
                    }
                    char * returnString = stringSlicer(string, start, end);
                    // Check to make sure this is a Param and is a valid one... if it's not, then return Error
                    bool foundEqule = false;
                    for (int j = 0; j < strlen(returnString); j++) {
                        if (returnString[j] == equle) {
                            if ((j >= 1) && (j < strlen(returnString))) {
                                foundEqule = true;
                            }
                        }
                    }
                    if (foundEqule == false) {
                        free(returnString);
                        deleteProperty(ptr);
                        return INV_PROP;
                    }

                    Parameter * ptrToParam = NULL;

                    VCardErrorCode temp = paramProcess(&returnString, &ptrToParam);

                    if (temp != OK) {
                        free(returnString);
                        deleteProperty(ptr);
                        return temp;
                    }

                    insertBack(ptr->parameters, (void*)ptrToParam);

                    if (returnString != NULL) {
                        free(returnString);
                        returnString = NULL;
                    }
                    firstParamFound = true;
                    start = i+1;
                }
            }
        } 
    }
    insertBack((*obj)->optionalProperties, (void*)ptr);
    return OK;
}

VCardErrorCode paramProcess(char ** returnString, Parameter ** ptr) {
    Parameter * parameterStruct = malloc(sizeof(Parameter) + sizeof(char) * 200);
    *ptr = parameterStruct;
    int paramStart = 0;
    int paramEnd = 0;  

    char equle = '=';

    for (int j = 0; j < strlen(*returnString); j++) {
        if ((*returnString)[j] == equle) {
            paramEnd = j;
        }
    }

    char * paramName = stringSlicer(returnString, paramStart, paramEnd);

    char * namePtr = parameterStruct->name;

    strcpy(namePtr, paramName);
    free(paramName);

    paramStart = paramEnd+1;
    paramEnd = strlen(*returnString);
    

    char * paramValue = stringSlicer(returnString, paramStart, paramEnd);
    int paramSize = strlen(paramValue);
    if (paramSize <= 0) {
        free(paramValue);
        free(parameterStruct);
        return INV_PROP;
    }

    char * valuePtr = parameterStruct->value;

    strcpy(valuePtr, paramValue);
    free(paramValue);
    
    return OK;
}

VCardErrorCode processFile(char *** fileArray, int arraySize, Card ** obj) {
    VCardErrorCode errorCode = OK;

    int fnString = -1;
    int bdayString = -1;
    int anniversaryString = -1;

    // First check to see if the End Line carrage return and New line characters exist or not.
    // Here we will remove the carrage return and newline character too!
    for (int i = 0; i < arraySize; i++) {
        int len = strlen((*fileArray)[i]);
        if (((*fileArray)[i][len-2] == '\r') && ((*fileArray)[i][len-1] == '\n')) {
            (*fileArray)[i][len-2] = '\0';
            (*fileArray)[i][len-1] = '\0';
        }
        else {
            errorCode = INV_PROP;
            return errorCode;
        }
    }

    char * copyString = NULL;
    char * string = NULL;

    // Now we will find the arrays that contain our main REQUIRED strings.
    // To do this, we will create a copy of the original string, then lowercase it and compare the chars.
    for (int i = 0; i < arraySize; i++) {
        string = (*fileArray)[i];
        int stringLen = strlen(string);
        copyString = malloc(sizeof(char) * stringLen + 1);
        strcpy(copyString, string);

        for(int j = 0; j < stringLen; j++) {
            copyString[j] = tolower(copyString[j]);
        }

        if ((copyString[0] == 'f') && (copyString[1] == 'n') && ((copyString[2] == ':') || (copyString[2] == ';') )) {
            fnString = i;
        }

        if ((copyString[0] == 'b') && (copyString[1] == 'd') && (copyString[2] == 'a') && (copyString[3] == 'y') && ((copyString[4] == ':') || (copyString[4] == ';') )) {
            bdayString = i;
        }

        if ((copyString[0] == 'a') && (copyString[1] == 'n') && (copyString[2] == 'n') && (copyString[3] == 'i') && (copyString[4] == 'v') && (copyString[5] == 'e') && (copyString[6] == 'r') && (copyString[7] == 's') && (copyString[8] == 'a') && (copyString[9] == 'r') && (copyString[10] == 'y') && ((copyString[11] == ':') || (copyString[11] == ';') )) {
            anniversaryString = i;
        }
        free(copyString);  
        copyString = NULL; 
    }

    // Here we will do our main processing.
    for (int i = 2; i < arraySize-1; i++) {
        if (i == fnString) {
            bool colon = false;
            for (int j = 0; j < strlen((*fileArray)[i]); j++) {
                if ((*fileArray)[i][j] == ':') {
                    colon = true;
                }
            }
            if (colon == true) {
                errorCode = fnPropertyParser((&(*fileArray)[i]), obj);
                if (errorCode != OK) {
                    return errorCode;
                }
            }
            if (colon == false) {
                return INV_PROP;
            }
        }
        else if ((bdayString != -1) && (i == bdayString)) {
            // Changed this to colon is true because seems like the colon can be missing in BDAY and ANNI
            bool colon = true;
            /*
            for (int j = 0; j < strlen((*fileArray)[i]); j++) {
                if ((*fileArray)[i][j] == ':') {
                    colon = true;
                }
            }*/
            if (colon == true) {
                errorCode = dateTimeParser((&(*fileArray)[i]), &(*obj)->birthday);
                if (errorCode != OK) {
                        return errorCode;
                }
            }
            if (colon == false) {
                return INV_PROP;
            }
        }
        else if ((anniversaryString != -1) && (i == anniversaryString)) {
            bool colon = true;
            /*
            for (int j = 0; j < strlen((*fileArray)[i]); j++) {
                if ((*fileArray)[i][j] == ':') {
                    colon = true;
                }
            }*/
            if (colon == true) {
                errorCode = dateTimeParser((&(*fileArray)[i]), &(*obj)->anniversary);
                if (errorCode != OK) {
                    return errorCode;
                }
            }
            if (colon == false) {
                return INV_PROP;
            }
        }
        else {
            bool colon = false;
            for (int j = 0; j < strlen((*fileArray)[i]); j++) {
                if ((*fileArray)[i][j] == ':') {
                    colon = true;
                }
            }
            if (colon == true) {
                errorCode = optionalPropertyParser((&(*fileArray)[i]), obj);
                if (errorCode != OK) {
                    return errorCode;
                }
            }
            if (colon == false) {
                return INV_PROP;
            }
        }
    }
    return errorCode;
}

char* toStringOpProps(List * list){
	ListIterator iter = createIterator(list);
	char* str;

	str = (char*)malloc(sizeof(char));
	strcpy(str, "");

	void* elem;
	while((elem = nextElement(&iter)) != NULL){
		char* currDescr = list->printData(elem);
		int newLen = strlen(str)+50+strlen(currDescr);
		str = (char*)realloc(str, newLen);
		//===> Removing the newline char because it's messing with my string structure for printing the card
		//strcat(str, "\n");
		strcat(str, currDescr);
		free(currDescr);
	}
	return str;
}

char* printBdayAnni(void* toBePrinted, char * name) {
    DateTime * temp = (DateTime*)toBePrinted;
    if (temp == NULL) {
        char * non = malloc(sizeof(char)*10);
        strcpy(non, "");
        return non;
    }

    bool value = temp->isText;

    if(value == true) {
        char * result = malloc(sizeof(char) * (strlen(temp->text) + 30 + strlen(name)));
        strcpy(result, name);
        strcat(result,";VALUE=text:");
        strcat(result, temp->text);
        strcat(result, "\r\n");
        return result;
    }
    
    bool timeString = false;
    bool dateString = false;
    bool isUTC = false;

    int len1 = strlen(temp->date);
    if (len1 > 2) {
        dateString = true;
    }

    int len2 = strlen(temp->time);
    if(len2 > 1) {
        timeString = true;
    }

    isUTC = temp->UTC;

    int finalLen = len1 + len2 + 40 + strlen(name);

    char * finalString = malloc(sizeof(char) * finalLen);

    if (dateString == true) {
        strcpy(finalString, name);
        strcat(finalString, ":");
        strcat(finalString, temp->date);
        if (timeString == true) {
            strcat(finalString, "T");
            strcat(finalString, temp->time);
            if (isUTC == true) {
                strcat(finalString, "Z");
            }
            strcat(finalString, "\r\n");
            return finalString;
        } else {
            strcat(finalString, "\r\n");
            return finalString;
        }
    }

    if (timeString == true) {
        strcpy(finalString, name);
        strcat(finalString, ":");
        strcat(finalString, "T");
        strcat(finalString, temp->time);
        if (isUTC == true) {
            strcat(finalString, "Z");
        }
        strcat(finalString, "\r\n");
        return finalString;
    }

    return finalString;
}

VCardErrorCode propertyObjCheck(Property * ptr) {
    if (ptr == NULL) {
        return INV_PROP;
    }

    char * name = ptr->name;
    if (name == NULL) {
        return INV_PROP;
    }
    int nameLen = strlen(name);

    char * group = ptr->group;
    if (group == NULL) {
        return INV_PROP;
    }

    List * param = ptr->parameters;
    if (param == NULL) {
        return INV_PROP;
    }
    int paramLen = getLength(param);

    List * vals = ptr->values;
    if (vals == NULL) {
        return INV_PROP;
    }
    int valsLen = getLength(vals);

    // If the name is an empty string
    if(nameLen <= 0) {
        return INV_PROP;
    }

    // Now check Param.
    ListIterator paramIter = createIterator(param);
    for (int i = 0; i < paramLen; i++) {

        int paramNameLen = strlen(((Parameter*)(paramIter.current->data))->name);
        int paramValLen = strlen(((Parameter*)(paramIter.current->data))->value);

        if ((paramNameLen <= 0) || (paramValLen <= 0)) {
            return INV_PROP;
        }
        nextElement(&paramIter);
    }

    // Now check Value.
    if(valsLen <= 0) {
        return INV_PROP;
    }

    return OK;
}

VCardErrorCode datetimeCheck(DateTime * ptr) {
    VCardErrorCode error = OK;
    if (ptr == NULL) {
        return INV_DT;
    }
    bool isText = ptr->isText;
    bool isUTC = ptr->UTC;
    char * date = ptr->date;
    char * time = ptr->time;
    char * text = ptr->text;

    if (isText == true) {
        int textLen = strlen(text);
        int dateLen = strlen(date);
        int timeLen = strlen(time);

        if (isUTC == true) {
            return INV_DT;
        }

        if (textLen <= 0) {
            return INV_DT;
        }
        
        if ((dateLen > 0) || (timeLen > 0)) {
            return INV_DT;
        }
    }
    if (isText == false) {
        int textLen = strlen(text);
        int dateLen = strlen(date);
        int timeLen = strlen(time);
        if (textLen > 0) {
            return INV_DT;
        }
        
        if ((dateLen < 0) || (timeLen < 0)) {
            return INV_DT;
        }
    }
    return error;
}

VCardErrorCode propNameChecker (Property * ptr, Tags ** tags) {
    if (ptr == NULL) {
        return INV_PROP;
    }
    if(tags == NULL) {
        return OTHER_ERROR;
    }

    VCardErrorCode error = OK;
    
    char * name = ptr->name;
    int nameLen = strlen(name);
    // This would mean the string is empty
    if (nameLen  == 0) {
        return INV_PROP;
    }
    // This would target tags: n
    if (nameLen == 1) {
        if(name[0] == 'N') {
            (*tags)->n = (*tags)->n + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 5) {
                return INV_PROP;
            }
        }
        else {
            return INV_PROP;
        }
    }
    // This would target tags: FN TZ
    else if (nameLen == 2) {
        if ((name[0] == 'F') && (name[1] == 'N')) {
            (*tags)->fn = (*tags)->fn + 1;            
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'T') && (name[1] == 'Z')) {
            (*tags)->tz = (*tags)->tz + 1;            
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else {
            return INV_PROP;
        }
    }
    // This would target (*tags): End XML ADR TEL GEO ORG REV UID URL KEY
    else if (nameLen == 3) {
        if ((name[0] == 'E') && (name[1] == 'N') && (name[2] == 'D')) {
            // This should not exist in the Optional Properties!
            return INV_PROP;
        }
        else if ((name[0] == 'X') && (name[1] == 'M') && (name[2] == 'L')) {
            (*tags)->xml = (*tags)->xml + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'A') && (name[1] == 'D') && (name[2] == 'R')) {
            (*tags)->adr = (*tags)->adr + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 7) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'T') && (name[1] == 'E') && (name[2] == 'L')) {
            (*tags)->tel = (*tags)->tel + 1;
            int valuesLen = getLength(ptr->values);
            if ((valuesLen <= 0) && (valuesLen >= 3)) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'G') && (name[1] == 'E') && (name[2] == 'O')) {
            (*tags)->geo = (*tags)->geo + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'O') && (name[1] == 'R') && (name[2] == 'G')) {
            (*tags)->org = (*tags)->org + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'R') && (name[1] == 'E') && (name[2] == 'V')) {
            (*tags)->rev = (*tags)->rev + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'U') && (name[1] == 'I') && (name[2] == 'D')) {
            (*tags)->uid = (*tags)->uid + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'U') && (name[1] == 'R') && (name[2] == 'L')) {
            (*tags)->url = (*tags)->url + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'K') && (name[1] == 'E') && (name[2] == 'Y')) {
            (*tags)->key = (*tags)->key + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else {
            return INV_PROP;
        }
    }
    // This would target tags: Kind BDAY IMPP LANG ROLE LOGO NOTE
    else if (nameLen == 4) {
        if ((name[0] == 'K') && (name[1] == 'I') && (name[2] == 'N') && (name[3] == 'D')) {  
            (*tags)->kind = (*tags)->kind + 1;
            int valuesLen = getLength(ptr->values);
            if ((*tags)->kind > 1) {
                return INV_PROP;
            }
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'B') && (name[1] == 'D') && (name[2] == 'A') && (name[3] == 'Y')) {  
            (*tags)->bday = (*tags)->bday + 1;
        }
        else if ((name[0] == 'I') && (name[1] == 'M') && (name[2] == 'P') && (name[3] == 'P')) {   
            (*tags)->impp = (*tags)->impp + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'L') && (name[1] == 'A') && (name[2] == 'N') && (name[3] == 'G')) {   
            (*tags)->lang = (*tags)->lang + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'R') && (name[1] == 'O') && (name[2] == 'L') && (name[3] == 'E')) {   
            (*tags)->role = (*tags)->role + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'L') && (name[1] == 'O') && (name[2] == 'G') && (name[3] == 'O')) {   
            (*tags)->logo = (*tags)->logo + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'N') && (name[1] == 'O') && (name[2] == 'T') && (name[3] == 'E')) {   
            (*tags)->note = (*tags)->note + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else {
            printf("I was here!\n");
            return INV_PROP;
        }
    }
    // This would target tags: Begin PHOTO EMAIL TITLE SOUND FBURL
    else if (nameLen == 5) {
        if ((name[0] == 'B') && (name[1] == 'E') && (name[2] == 'G') && (name[3] == 'I') && (name[4] == 'N')) {
            // Should not exist in optional oroperties
            return INV_PROP;
        }
        else if ((name[0] == 'P') && (name[1] == 'H') && (name[2] == 'O') && (name[3] == 'T') && (name[4] == 'O')) {
            (*tags)->photo = (*tags)->photo + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'E') && (name[1] == 'M') && (name[2] == 'A') && (name[3] == 'I') && (name[4] == 'L')) {
            (*tags)->email = (*tags)->email + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'T') && (name[1] == 'I') && (name[2] == 'T') && (name[3] == 'L') && (name[4] == 'E')) {
            (*tags)->title = (*tags)->title + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'F') && (name[1] == 'B') && (name[2] == 'U') && (name[3] == 'R') && (name[4] == 'L')) {
            (*tags)->fburl = (*tags)->fburl + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'S') && (name[1] == 'O') && (name[2] == 'U') && (name[3] == 'N') && (name[4] == 'D')) {
            (*tags)->sound = (*tags)->sound + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else {
            return INV_PROP;
        }
    }
    // This would target (*tags): Source GENDER MEMBER PRODID CALURI
    else if (nameLen == 6) {
        if ((name[0] == 'S') && (name[1] == 'O') && (name[2] == 'U') && (name[3] == 'R') && (name[4] == 'C') && (name[5] == 'E')) {
            (*tags)->source = (*tags)->source + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'G') && (name[1] == 'E') && (name[2] == 'N') && (name[3] == 'D') && (name[4] == 'E') && (name[5] == 'R')) {
            (*tags)->gender = (*tags)->gender + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'M') && (name[1] == 'E') && (name[2] == 'M') && (name[3] == 'B') && (name[4] == 'E') && (name[5] == 'R')) {
            (*tags)->member = (*tags)->member + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'P') && (name[1] == 'R') && (name[2] == 'O') && (name[3] == 'D') && (name[4] == 'I') && (name[5] == 'D')) {
            (*tags)->prodid = (*tags)->prodid + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'C') && (name[1] == 'A') && (name[2] == 'L') && (name[3] == 'U') && (name[4] == 'R') && (name[5] == 'I')) {
            (*tags)->caluri = (*tags)->caluri + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else {
            return INV_PROP;
        }
    } 
    // This would target (*tags): RELATED VERSION
    else if (nameLen == 7) {
        if ((name[0] == 'R') && (name[1] == 'E') && (name[2] == 'L') && (name[3] == 'A') && (name[4] == 'T') && (name[5] == 'E') && (name[6] == 'D')) {
            (*tags)->related = (*tags)->related + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else if ((name[0] == 'V') && (name[1] == 'E') && (name[2] == 'R') && (name[3] == 'S') && (name[4] == 'I') && (name[5] == 'O') && (name[6] == 'N')) {
            // This should not exist in Optional Properties
            return INV_CARD;
        }
        else {
            return INV_PROP;
        }

    }
    // This would target (*tags): NICKNAME
    else if (nameLen == 8) {
        if ((name[0] == 'N') && (name[1] == 'I') && (name[2] == 'C') && (name[3] == 'K') && (name[4] == 'N') && (name[5] == 'A') && (name[6] == 'M') && (name[7] == 'E')) {
            (*tags)->nickname = (*tags)->nickname + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen >= 1) {
                return INV_PROP;
            }
        }
        else {
            return INV_PROP;
        }
    }
    // This would target (*tags): categories
    else if (nameLen == 10) {
        if ((name[0] == 'C') && (name[1] == 'A') && (name[2] == 'T') && (name[3] == 'E') && (name[4] == 'G') && (name[5] == 'O') && (name[6] == 'R') && (name[7] == 'I') && (name[8] == 'E') && (name[9] == 'S')) {
            (*tags)->categories = (*tags)->categories + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else {
            return INV_PROP;
        }
    }
    // This would target (*tags): ANNIVERSARY CALANDERURI
    else if (nameLen == 11) {
        if ((name[0] == 'A') && (name[1] == 'N') && (name[2] == 'N') && (name[3] == 'I')  && (name[4] == 'V') && (name[5] == 'E')   && (name[6] == 'R') && (name[7] == 'S') && (name[8] == 'A') && (name[9] == 'R') && (name[10] == 'Y')) {
            (*tags)->anniversary = (*tags)->anniversary + 1;
        }
        else if ((name[0] == 'C') && (name[1] == 'A') && (name[2] == 'L') && (name[3] == 'A') && (name[4] == 'N') && (name[5] == 'D')   && (name[6] == 'E') && (name[7] == 'R')  && (name[8] == 'U')  && (name[9] == 'R') && (name[10] == 'I')) {
            (*tags)->calanderuri = (*tags)->calanderuri + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 1) {
                return INV_PROP;
            }
        }
        else {
            return INV_PROP;
        }
    }
    // This would target (*tags): CLIENTPIDMAP
    else if (nameLen == 12) {
        if ((name[0] == 'C') && (name[1] == 'L') && (name[2] == 'I')  && (name[3] == 'E') && (name[4] == 'N')  && (name[5] == 'T') && (name[6] == 'P')   && (name[7] == 'I') && (name[8] == 'D') && (name[9] == 'M') && (name[10] == 'A') && (name[11] == 'P')) {
            (*tags)->clientpidmap = (*tags)->clientpidmap + 1;
            int valuesLen = getLength(ptr->values);
            if (valuesLen != 2) {
                return INV_PROP;
            }
        }
        else {
            return INV_PROP;
        }
    }
    else {
        return INV_PROP;
    }
    return error;
}

char* cardToJson (Card* card) {
    if (card == NULL) {
        char * returnString = malloc(sizeof(char) * 30);
        strcpy(returnString, "Oops! Something went wrong!");
        return returnString;
    }

    VCardErrorCode error = validateCard(card);
    if (error != OK) {
        //char * returnString = malloc(sizeof(char) * 30);
        //strcpy(returnString, "Oops! Something went wrong!");
        //return returnString;
        return NULL;
    }

    Property * fn = card->fn;
    List * opProp = card->optionalProperties;
    DateTime * bday = card->birthday;
    DateTime * anni = card->anniversary;

    char * returnString = malloc(sizeof(char) * 10);
    strcpy(returnString, "[");

    char * fnString = propToJSON(fn);
    returnString = realloc(returnString, strlen(returnString) + strlen(fnString) + 50);
    strcat(returnString, fnString);
    free(fnString);

    int iterLen = getLength(opProp);
    if (iterLen > 0) {
        ListIterator iter = createIterator(opProp);
        for (int i = 0; i < iterLen; i++) {
            Property * temp = (Property*)(iter.current->data);
            char * opPropString = propToJSON(temp);
            returnString = realloc(returnString, strlen(returnString) + strlen(opPropString) + 50);
            strcat(returnString, ",");
            strcat(returnString, opPropString);
            free(opPropString);
            nextElement(&iter);
        }
    }

    if (bday != NULL) {
        char * bdayString = dtToJSON(bday);
        returnString = realloc(returnString, strlen(returnString) + strlen(bdayString) + 50);
        strcat(returnString, ",");
        strcat(returnString, bdayString);
        free(bdayString);
    }

    if (anni != NULL) {
        char * anniString = dtToJSON(anni);
        returnString = realloc(returnString, strlen(returnString) + strlen(anniString) + 50);
        strcat(returnString, ",");
        strcat(returnString, anniString);
        free(anniString);
    }

    strcat(returnString, "]");
    return returnString;
}