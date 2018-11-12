#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "VCardHelper.h"
#include "VCardParser.h"
#include "LinkedListAPI.h"

// *******************************
// Major Card Operations
// *******************************
VCardErrorCode createCard(char* fileName, Card** newCardObject) {

    VCardErrorCode errorCode;

    // Double pointer that points to the file in array form
    char **fileArray = NULL;

    // Function call that returns the size of the array
    int preProcessResult = preProcessFile(fileName, &fileArray);

    // Check to see if the file wrong or empty.
    if (preProcessResult <= 0) {
        deleteSavedFile(&fileArray, preProcessResult);
        errorCode = INV_FILE;
        return errorCode;
    }

    // Check to see if the file has bare minimum requirments
    int initialFileStatus = initialCheckFile(&fileArray, preProcessResult);

    if (initialFileStatus == false) {
        deleteSavedFile(&fileArray, preProcessResult);
        errorCode = INV_CARD;
        return errorCode;
    }

    // Since File should have all necessary variables, we can parse it into the Vcard!

    // *******************************************************************************
    // Card Object is created here! 
    // *******************************************************************************
    *newCardObject = malloc(sizeof(Card));

    (*newCardObject)->fn = malloc(sizeof(Property));
    (*newCardObject)->fn->parameters = initializeList(&printParameter, &deleteParameter, &compareParameters);
    (*newCardObject)->fn->values = initializeList(&printValue, &deleteValue, &compareValues);
    (*newCardObject)->fn->name = malloc(sizeof(char));
    // ===> THIS WAS MALLOC'ed TO FULFILL THE REQUIRMENT OF NOT BEING NULL
    (*newCardObject)->fn->group = malloc(sizeof(char) * 1);
    strcpy((*newCardObject)->fn->group, "");


    (*newCardObject)->optionalProperties = initializeList(&printProperty, &deleteProperty, &compareProperties);
    (*newCardObject)->birthday = NULL;
    (*newCardObject)->anniversary = NULL;
    // *******************************************************************************
    // *******************************************************************************

    errorCode = processFile(&fileArray, preProcessResult, newCardObject);

    if (errorCode != OK) {
        deleteCard(*newCardObject);
        // Delete the Saved array before leaving!
        deleteSavedFile(&fileArray, preProcessResult);
        return errorCode;
    }
    //deleteCard(*newCardObject);
    // Delete the Saved array before leaving!
    deleteSavedFile(&fileArray, preProcessResult);
    // In all cases return OK for now.
    return errorCode;
}

VCardErrorCode writeCard(const char* fileName, const Card* obj) {

    FILE * ptr = NULL;
    bool validFileExt = false;
    VCardErrorCode error = OK;

    if (fileName == NULL) {
        return WRITE_ERROR;
    }

    if (obj == NULL) {
        return WRITE_ERROR;
    }

    char * fileNametemp = malloc(sizeof(char) * strlen(fileName) + 1);
    strcpy(fileNametemp, fileName);

    validFileExt = validateFileExtention(fileNametemp);
    error = validateCard(obj);
    if (error != OK) {
        free(fileNametemp);
        return error;
    }

    if (validFileExt == false) {
        return WRITE_ERROR;
    }
    free(fileNametemp);

    ptr = fopen(fileName,"w+");

    if (ptr == NULL) {
        return WRITE_ERROR;
    }

    char * toPrint = printCard(obj);
    int temp = strlen(toPrint);
    if (temp <= 1) {
        fclose(ptr);
        return WRITE_ERROR;
    }

    char * beginVcard = malloc(sizeof(char) * 30);
    strcpy(beginVcard, "BEGIN:VCARD\r\n");

    char * cardVersion = malloc(sizeof(char) * 30);
    strcpy(cardVersion, "VERSION:4.0\r\n");

    char * endVcard = malloc(sizeof(char) * 30);
    strcpy(endVcard, "END:VCARD\r\n");

    int fileLength = strlen(toPrint) + strlen(beginVcard) + strlen(cardVersion) + strlen(endVcard) + 30;
    char * fileToPrint = malloc(sizeof(char) * fileLength);

    strcpy(fileToPrint, beginVcard);
    strcat(fileToPrint, cardVersion);
    strcat(fileToPrint, toPrint);
    strcat(fileToPrint, endVcard);

    free(beginVcard);
    free(cardVersion);
    free(toPrint);
    free(endVcard);

    fprintf(ptr, "%s", fileToPrint);
    fclose(ptr);
    free(fileToPrint);

    return error;
}

VCardErrorCode validateCard(const Card* obj) {
    if (obj == NULL) {
        return INV_CARD;
    }

    Property * fn = obj->fn;
    List * optionalProperties = obj->optionalProperties;
    DateTime * bday = obj->birthday;
    DateTime * anni = obj->anniversary;

    VCardErrorCode error = OK;

    if (fn == NULL) {
        return INV_CARD;
    }
    error = propertyObjCheck(fn);
    if (error != OK) {
        return error;
    }
    int listLen = 0;
    if (optionalProperties != NULL) {
        ListIterator opPropIter = createIterator(optionalProperties);
        listLen = getLength(optionalProperties);

        for (int i = 0; i < listLen; i++) {
            error = propertyObjCheck((Property*)(opPropIter.current->data));
            if (error != OK) {
                return error;
            }
            nextElement(&opPropIter);
        }
    } else {
        return INV_CARD;
    }

    if (bday != NULL) {
        error = datetimeCheck(bday);
        if (error != OK) {
            return error;
        }
    }

    if (anni != NULL) {
        error = datetimeCheck(anni);
        if (error != OK) {
            return error;
        }
    }

    Tags * tags = malloc(sizeof(Tags));
    tags->begin = 0;
    tags->end = 0;
    tags->version = 0;
    tags->fn = 0;
    tags->kind = 0;
    tags->n = 0;
    tags->bday = 0;
    tags->anniversary = 0;
    tags->gender = 0;
    tags->prodid = 0;
    tags->rev = 0;
    tags->uid = 0;
    tags->source = 0;
    tags->xml = 0;
    tags->nickname = 0;
    tags->photo = 0;
    tags->adr = 0;
    tags->tel = 0;
    tags->email = 0;
    tags->impp = 0;
    tags->lang = 0;
    tags->tz = 0;
    tags->geo = 0;
    tags->title = 0;
    tags->role = 0;
    tags->logo = 0;
    tags->org = 0;
    tags->member = 0;
    tags->related = 0;
    tags->categories = 0;
    tags->note = 0;
    tags->sound = 0;
    tags->clientpidmap = 0;
    tags->url = 0;
    tags->key = 0;
    tags->fburl = 0;
    tags->calanderuri = 0;
    tags->caluri = 0;

    error = propNameChecker(obj->fn, &tags);
    if (error != OK) {
        free(tags);
        return error;
    }

    ListIterator optionalPropIter = createIterator(optionalProperties);
    for (int i = 0; i < listLen; i++) {
        error = propNameChecker((Property*)(optionalPropIter.current->data), &tags);
        if (error != OK) {
            free(tags);
            return error;
        }
        nextElement(&optionalPropIter);
    }

    if (tags->fn <= 0) {
        free(tags);
        return INV_PROP;
    } 
    free(tags);
    return error;
}

// *******************************
// Comparing
// *******************************

int compareProperties(const void* first,const void* second) {
    if (first == NULL) {
        return -1;
    }
    if (second == NULL) {
        return -1;
    }
    Property * firstStruct = (Property*) first;
    Property * secondStruct = (Property*) second;
    if (firstStruct == NULL) {
        return -1;
    }
    if (secondStruct == NULL) {
        return -1;
    }
    int i = 0;
    int result = 0;

    if (firstStruct->name == NULL) {
        return -1;
    }
    if (secondStruct->name == NULL) {
        return -1;
    }

    int fNlength = strlen(firstStruct->name);
    int sNlength = strlen(secondStruct->name);

    char * firstName = malloc(sizeof(char) * fNlength + 1);
    char * secondName = malloc(sizeof(char)*sNlength + 1);

    strcpy(firstName, firstStruct->name);
    strcpy(secondName, secondStruct->name);

    while(firstName[i]) {
        putchar(tolower(firstName[i]));
        i++;
    }

    i = 0;

    while(secondName[i]) {
        putchar(tolower(secondName[i]));
        i++;
    }
    
    result = strcmp(firstName, secondName);

    free(firstName);
    free(secondName);

    return result; 
}

int compareParameters(const void* first,const void* second) {
    Parameter * firstStruct = (Parameter*) first;
    Parameter * secondStruct = (Parameter*) second;
    if (firstStruct == NULL) {
        return -1;
    }
    if (secondStruct == NULL) {
        return -1;
    }
    int i = 0;
    int result = 0;

    if (firstStruct->name == NULL) {
        return -1;
    }
    if (secondStruct->name == NULL) {
        return -1;
    }

    int fNlength = strlen(firstStruct->name);
    int sNlength = strlen(secondStruct->name);

    char * firstName = malloc(sizeof(char) * fNlength + 1);
    char * secondName = malloc(sizeof(char)*sNlength + 1);

    strcpy(firstName, firstStruct->name);
    strcpy(secondName, secondStruct->name);

    while(firstName[i]) {
        putchar(tolower(firstName[i]));
        i++;
    }

    i = 0;

    while(secondName[i]) {
        putchar(tolower(secondName[i]));
        i++;
    }
    
    result = strcmp(firstName, secondName);

    free(firstName);
    free(secondName);

    return result; 
}

int compareValues(const void* first,const void* second) {
    char * stringOne = (char*)first;
    char * stringTwo = (char*)second;
    if ((stringOne == NULL) || (stringTwo == NULL)) {
        return -1;
    }
    int result = 0;
    int i = 0;

    while(stringOne[i]) {
        putchar(tolower(stringOne[i]));
        i++;
    }

    i = 0;

    while(stringTwo[i]) {
        putchar(tolower(stringTwo[i]));
        i++;
    }
    
    result = strcmp(stringOne, stringTwo);

    return result;
}

int compareDates(const void* first,const void* second) {
    DateTime * firstStruct = (DateTime*)first;
    DateTime * secondStruct = (DateTime*)second;
    if ((firstStruct == NULL) || (secondStruct == NULL)) {
        return -1;
    }
    bool firstStructisText = firstStruct->isText;
    bool secondStructisText = secondStruct->isText;

    int i = 0;
    int result = 0;

    if(firstStructisText && secondStructisText) {
        char * stringOne = firstStruct->text;
        char * stringTwo = secondStruct->text;

        char * stringOneCopy = malloc(sizeof(char) * strlen(stringOne) + 1);
        char * stringTwoCopy = malloc(sizeof(char) * strlen(stringTwo) + 1);

        strcpy(stringOneCopy, stringOne);
        strcpy(stringTwoCopy, stringTwo);

        while(stringOne[i]) {
            putchar(tolower(stringOne[i]));
            i++;
        }

        i = 0;

        while(stringTwo[i]) {
            putchar(tolower(stringTwo[i]));
            i++;
        }
    
        result = strcmp(stringOne, stringTwo);

        free(stringOneCopy);
        free(stringTwoCopy);
        return result;
    }
    if (firstStructisText && !secondStructisText) {
        return 1;
    }
    if (!firstStructisText && secondStructisText) {
        return -1;
    }
    return result;
}

// *******************************
// Deleting
// *******************************

void deleteProperty(void* toBeDeleted) {
    Property * temp = (Property*)toBeDeleted;
    if (temp == NULL) {
        free(temp);
        return;
    }
    if (temp->name != NULL) {
        free(temp->name);
    }
    if (temp->group != NULL) {
        free(temp->group);
    }
    if (temp->parameters != NULL) {
        freeList(temp->parameters);
    }
    if (temp->values != NULL) {
        freeList(temp->values);
    }
    free(temp);
}

void deleteParameter(void* toBeDeleted) {
    Parameter * temp = (Parameter*)toBeDeleted;
    if (temp != NULL) {
        free(temp);
    }
}

void deleteValue(void* toBeDeleted) {
    char * temp = (char*)toBeDeleted;
    if (temp != NULL) {
        free(temp);
    }
}

void deleteDate(void* toBeDeleted) {
    DateTime * temp = (DateTime*)toBeDeleted;
    if (temp != NULL) {
        free(temp);
    }
}

void deleteCard(Card* obj) {
    Card * temp = obj;
    if (temp == NULL) {
        return;
    }

    if (temp == NULL) {
        return;
    }
    else {
        if (temp->fn != NULL) {
            deleteProperty((void*)temp->fn);
        }
        if (temp->optionalProperties != NULL) {
            freeList(temp->optionalProperties);
        }
        if (temp->birthday != NULL) {
            deleteDate((void*)temp->birthday);
        }
        if (temp->anniversary != NULL) {
            deleteDate((void*)temp->anniversary);
        }
    }
    free(obj);
}

// *******************************
// Printing
// *******************************

char* printProperty(void* toBePrinted) {
    Property * block = (Property*) toBePrinted;
    if (block == NULL) {
        char * non = malloc(sizeof(char)*10);
        strcpy(non, "");
        return non;
    }

    int nameLen = strlen(block->name);
    int groupLen = strlen(block->group);

    char * parameterString = toString((void*)(block->parameters));
    char * valuesString = toString((void*)(block->values));

    int parameterStringLen = 0;
    int valuesStringLen = 0;

    if ((parameterString != NULL) && (getLength((void*)(block->parameters)) != 0)) {
        parameterStringLen = strlen(parameterString);
    } else {
        free(parameterString);
        parameterString = NULL;
    }

    if ((valuesString != NULL) && ((getLength((void*)(block->values)) != 0))) {
        valuesStringLen = strlen(valuesString);
    } else {
        free(valuesString);
        valuesString = NULL;
    }

    int totalLength = nameLen + groupLen + parameterStringLen + valuesStringLen + 20;

    bool trip = false;

    char * finalString = malloc(sizeof(char)* totalLength);

    if((strlen(block->group) > 3) && (block->group != NULL)) {
        strcpy(finalString, block->group);
        strcat(finalString, ".");
        trip = true;
    }

    if (trip == true) {
        strcat(finalString, block->name);
    } else {
        strcpy(finalString, block->name);
    }
    
    if(parameterString != NULL) {
        strcat(finalString, ";");
        strcat(finalString, parameterString);
        free(parameterString);
        parameterString = NULL;
    }
    
    if (valuesString != NULL) {
        strcat(finalString, ":");       
        strcat(finalString, valuesString);
        free(valuesString);
    }
    strcat(finalString, "\r\n");
    return finalString;
}

char* printParameter(void* toBePrinted) {
    Parameter * block = (Parameter*) toBePrinted;
    if (block == NULL) {
        char * non = malloc(sizeof(char)*10);
        strcpy(non, "");
        return non;
    }
    char * equle = "=";

    int nameLen = strlen(block->name);
    int valueLen = strlen(block->value);

    if ((nameLen == 0)) {
        char * placeholderString = malloc(sizeof(char) * 10);
        strcpy (placeholderString, "");
        return placeholderString;
    }

    int totalLength = nameLen + valueLen + 20;
    char * finalString = malloc(sizeof(char) * totalLength);

    strcpy(finalString, block->name);
    strcat(finalString, equle);
    strcat(finalString, block->value);
    return finalString;
}

char* printValue(void* toBePrinted) {
    char * temp = (char*)toBePrinted;
    int length = strlen(temp);
    char * finalString = malloc(sizeof(char) * length+ 10);
    strcpy(finalString, temp);
    strcat(finalString, "");
    return finalString;
}

char* printDate(void* toBePrinted) {
    DateTime * temp = (DateTime*)toBePrinted;
    char * delim = "";
    if (temp == NULL) {
        char * non = malloc(sizeof(char)*10);
        strcpy(non, "");
        return non;
    }

    bool value = temp->isText;

    if(value == true) {
        char * result = malloc(sizeof(char) * strlen(temp->text) + 20);
        strcpy(result, temp->text);
        return result;
    }
    
    int len1 = strlen(temp->date);
    int len2 = strlen(temp->time);
    int finalLen = len1 + len2 + 20;

    char * finalString = malloc(sizeof(char) * finalLen);

    strcpy(finalString, temp->date);
    strcat(finalString, delim);
    strcat(finalString, temp->time);
    strcat(finalString, delim);

    return finalString;
}

char* printCard(const Card* obj) {  

    if (obj == NULL) {
        return NULL;
    }

    int propLen = 0;
    int addPropLen = 0;
    int bdayLen = 0;
    int anniLen = 0;

    char * prop = printProperty(obj->fn);
    propLen = strlen(prop);

    char * addProp = toStringOpProps(obj->optionalProperties);

    addPropLen = strlen(addProp);

    char * bday = NULL;
    char * anniver = NULL;

    if (obj->birthday != NULL) {
        char * name = malloc(sizeof(char) * 20);
        strcpy(name, "BDAY");
        bday = printBdayAnni(obj->birthday, name);   
        bdayLen = strlen(bday);
        free(name);
    }
    if (obj->anniversary != NULL) {
        char * name = malloc(sizeof(char) * 20);
        strcpy(name, "ANNIVERSARY");
        anniver = printBdayAnni(obj->anniversary, name);
        anniLen = strlen(anniver);
        free(name);
    }

    // Remember to consider the delim size in the malloc'ed array size
    int totalLen = propLen + addPropLen + bdayLen + anniLen + 20;

    char * finalString = malloc(sizeof(char) * totalLen);
    for (int i = 0; i < totalLen; i++) {
        finalString[i] = '\0';
    }

    strcpy(finalString, prop);
    free(prop);

    strcat(finalString, addProp);
    free(addProp);

    if (obj->birthday != NULL) {
        strcat(finalString, bday);
        free(bday);
    }
    
    if (obj->anniversary != NULL) {
        strcat(finalString, anniver);
        free(anniver);
    }

    return finalString;    
}

char* printError(VCardErrorCode err) {
    char * result = malloc(sizeof(char) * 30);
    if (err == OK) {
        strcpy(result, "OK\n");
        return result;
    }
    if (err == INV_FILE) {
        strcpy(result, "INV_FILE\n");
        return result;
    }
    if (err == INV_CARD) {
        strcpy(result, "INV_CARD\n");
        return result;
    }
    if (err == INV_PROP) {
        strcpy(result, "INV_PROP\n");
        return result;
    }
    if (err == WRITE_ERROR) {
        strcpy(result, "WRITE_ERROR\n");
        return result;
    }
    if (err == OTHER_ERROR) {
        strcpy(result, "OTHER_ERROR\n");
        return result;
    }
    if (err == INV_DT) {
        strcpy(result, "INV_DT\n");
        return result;       
    }
    strcpy(result, "Invalid error code\n");
    return result;
}

// *******************************
// JSON <-> STRING  
// *******************************

char* strListToJSON(const List* strList) {
    if (strList == NULL) {
        return NULL;
        //char * emptyString = malloc(sizeof(char) * 10);
        //strcpy(emptyString, "[]");
        //return emptyString;
    }

    int strListLen = 0;
    strListLen = strList->length;

    if (strListLen <= 0) {
        char * emptyString = malloc(sizeof(char) * 10);
        strcpy(emptyString, "[]");
        return emptyString;
    }
    
    char * jsonString = malloc(sizeof(char) * 10);
    strcpy(jsonString, "[");

    Node * listIter = strList->head;

    for (int i = 0; i < strListLen; i++) {
        char * valueTemp = ((char*)(listIter->data));
        int valueTempLen = strlen(valueTemp);

        char * value = malloc(sizeof(char) * valueTempLen+1);
        strcpy(value, valueTemp);

        int valueLen = strlen(value);

        char * tempString = malloc(sizeof(char) * valueLen + 5);

        if (jsonString[strlen(jsonString)-1] != '\"') {
            strcpy(tempString, "\"");
            strcat(tempString, value);
            strcat(tempString, "\"");
            free(value);
        } else {
            strcpy(tempString, ",");           
            strcat(tempString, "\"");
            strcat(tempString, value);
            strcat(tempString, "\"");
            free(value);
        }

        jsonString = realloc(jsonString, strlen(jsonString) + strlen(tempString) + 3);

        strcat(jsonString, tempString);
        
        // Not sure if doing this will break the program...
        free(tempString);

        listIter = listIter->next;
    }
    strcat(jsonString, "]");

    return jsonString;
}

List* JSONtoStrList(const char* str) {
    if (str == NULL) {
        return NULL;
    }

    if (strlen(str) <= 2) {
        return NULL;
    }

    List * valList = initializeList(&printValue, &deleteValue, &compareValues);

    int start = 0;
    int end = 0;

    int stringLen = strlen(str);
    char * tempStr = malloc(sizeof(char) * stringLen + 1);
    strcpy(tempStr, str);

    for (int i = 0; i < stringLen; i++) {
        if (tempStr[i] == '\"') {
            // This will record where to end.
            if ((start != 0) && (end == 0)) {
                end = i;
            }

            // This will record where to start.
            if ((start == 0) && (end == 0)) {
                start = i;
            }
            
            // This will cleave and place string in valList
            if ((start != 0) && (end != 0)) {
                if (end == start + 1) {
                    char * temp = malloc(sizeof(char) * 2);
                    strcpy(temp, "");
                    insertBack(valList, (void*)temp);
                    start = end = 0;
                } else {
                    char * temp = stringSlicer(&tempStr, start+1, end);
                    insertBack(valList, (void*)temp);
                    start = end = 0;
                }
            }
        }
    }
    int listLen = getLength(valList);
    if (listLen <= 0) {
        free(tempStr);
        freeList(valList);
        return NULL;
    }
    free(tempStr);
    return valList;
}

char* propToJSON(const Property* prop) {
    
    if (prop == NULL) {
        char * temp = malloc(sizeof(char) * 2);
        strcpy(temp, "");
        return temp;
    }

    char * groupTag = malloc(sizeof(char) * 20);
    strcpy(groupTag, "\"group\":");

    char * nameTag = malloc(sizeof(char) * 20);
    strcpy(nameTag, "\"name\":");

    char * valueTag = malloc(sizeof(char) * 20);
    strcpy(valueTag, "\"values\":");

    char * group = malloc(sizeof(char) * strlen(prop->group) + 1);
    strcpy(group, prop->group);
    int groupLen = strlen(group);

    char * groupCopy = malloc(sizeof(char) * groupLen + 20);
    strcpy(groupCopy, "\"");
    strcat(groupCopy, group);
    strcat(groupCopy, "\"");


    char * name = malloc(sizeof(char) * strlen(prop->name) + 1);
    strcpy(name, prop->name);
    int nameLen = strlen(name);
    char * nameCopy = malloc(sizeof(char) * nameLen + 20);
    strcpy(nameCopy, "\"");
    strcat(nameCopy, name);
    strcat(nameCopy, "\"");

    List * values = prop->values;
    char * valuesString = strListToJSON(values);
    int valuesLen = strlen(valuesString);

    char * returnString = malloc(sizeof(char) * (strlen(groupTag) + strlen(nameTag) + strlen(valueTag) + groupLen + nameLen + valuesLen + 20));
    
    strcpy(returnString, "{");   
    strcat(returnString, groupTag);
    strcat(returnString, groupCopy);
    strcat(returnString, ",");
    strcat(returnString, nameTag);
    strcat(returnString, nameCopy);
    strcat(returnString, ",");
    strcat(returnString, valueTag);
    strcat(returnString, valuesString);
    strcat(returnString, "}");

    free(groupTag);
    free(nameTag);
    free(valueTag);

    free(group);
    free(groupCopy);

    free(name);
    free(nameCopy);

    free(valuesString);

    return returnString;
}

Property* JSONtoProp(const char* str) {

    if(str == NULL) {
        return NULL;
    }

    if (strlen(str) <= 2) {
        return NULL;
    }

    char * tempStr = malloc(sizeof(char) * strlen(str) + 1);
    Property * returnProp = malloc(sizeof(Property));
    strcpy(tempStr, str);

    int stringLen = strlen(tempStr);

    for (int i = 0; i < stringLen; i++) {
        if ((tempStr[i] == '\"') && (tempStr[i+1] == 'g') && (tempStr[i+2] == 'r') && (tempStr[i+3] == 'o') && (tempStr[i+4] == 'u') && (tempStr[i+5] == 'p') && (tempStr[i+6] == '\"') && (tempStr[i+7] == ':')) {
            int start = i + 8;
            int end = 0;
            for (int j = start+1; j < stringLen; j++) {
                if (tempStr[j] == '\"') {
                    end = j;
                    break;
                }
            }
            if (end == start + 1) {
                char * temp = malloc(sizeof(char)*2);
                strcpy(temp, "");
                returnProp->group = temp;
            } else {
                if (end == 0) {
                    free(tempStr);
                    deleteProperty(returnProp);
                    return NULL;
                }
                char * temp = stringSlicer(&tempStr, start, end);
                returnProp->group = temp;
            }
        }
        if ((tempStr[i] == '\"') && (tempStr[i+1] == 'n') && (tempStr[i+2] == 'a') && (tempStr[i+3] == 'm') && (tempStr[i+4] == 'e') && (tempStr[i+5] == '\"') && (tempStr[i+6] == ':')) {
            int start = i + 7;
            int end = 0;
            for (int j = start+1; j < stringLen; j++) {
                if (tempStr[j] == '\"') {
                    end = j;
                    break;
                }
            }
            if (end == start + 1) {
                char * temp = malloc(sizeof(char)*2);
                strcpy(temp, "");
                returnProp->name = temp;
            } else {
                char * temp = stringSlicer(&tempStr, start+1, end);
                returnProp->name = temp;
            }
        }
        if ((tempStr[i] == '\"') && (tempStr[i+1] == 'v') && (tempStr[i+2] == 'a') && (tempStr[i+3] == 'l') && (tempStr[i+4] == 'u') && (tempStr[i+5] == 'e') && (tempStr[i+6] == 's') && (tempStr[i+7] == '\"') && (tempStr[i+8] == ':')) {
            int start = i + 9;
            int end = stringLen - 1;
            char * string = stringSlicer(&tempStr, start, end);
            List * ptr = JSONtoStrList(string);
            free(string);
            returnProp->values = ptr;
        }
    }
    free(tempStr);
    List * params = initializeList(&printParameter, &deleteParameter, &compareParameters);
    returnProp->parameters = params;

    return returnProp;
}

char* dtToJSON(const DateTime* prop) {
    if (prop == NULL) {
        char * temp = malloc(sizeof(char) * 2);
        strcpy(temp, "");
        return temp;
    }

    char * isText = malloc(sizeof(char) * 20);
    strcpy(isText, "\"isText\":");

    char * date = malloc(sizeof(char) * 20);
    strcpy(date, "\"date\":");

    char * curTime = malloc(sizeof(char) * 20);
    strcpy(curTime, "\"time\":");

    char * isUTC = malloc(sizeof(char) * 20);
    strcpy(isUTC, "\"isUTC\":");

    char * text = malloc(sizeof(char) * 20);
    strcpy(text, "\"text\":");

    char * trueText = malloc(sizeof(char) * 20);
    strcpy(trueText, "true");

    char * falseText = malloc(sizeof(char) * 20);
    strcpy(falseText, "false");

    char * propDate = malloc(sizeof(char) * strlen(prop->date) + 20);
    strcpy(propDate, "\"");
    strcat(propDate, prop->date);
    strcat(propDate, "\"");

    char * propTime = malloc(sizeof(char) * strlen(prop->time) + 20);
    strcpy(propTime, "\"");
    strcat(propTime, prop->time);
    strcat(propTime, "\"");

    char * propText = malloc(sizeof(char) * strlen(prop->text) + 20);
    strcpy(propText, "\"");
    strcat(propText, prop->text);
    strcat(propText, "\"");
    
    char * returnString = malloc(sizeof(char) * (strlen(isText) + strlen(date) + strlen(curTime) + strlen(isUTC) + strlen(trueText) + strlen(falseText) + strlen(prop->date) + strlen(prop->text) + strlen(prop->time) + 50));
    strcpy(returnString, "{");
    strcat(returnString, isText);
    if (prop->isText == true) {
        strcat(returnString, trueText);
    }
    if (prop->isText == false) {
        strcat(returnString, falseText);
    }
    strcat(returnString, ",");
    strcat(returnString, date);
    strcat(returnString, propDate);

    strcat(returnString, ",");
    strcat(returnString, curTime);
    strcat(returnString, propTime);

    strcat(returnString, ",");
    strcat(returnString, text);
    strcat(returnString, propText);

    strcat(returnString, ",");
    strcat(returnString, isUTC);
    if (prop->UTC == true) {
        strcat(returnString, trueText);
    }
    if (prop->UTC == false) {
        strcat(returnString, falseText);
    }
    strcat(returnString, "}");

    free(isText);
    free(date);
    free(curTime);
    free(isUTC);
    free(text);
    free(trueText);
    free(falseText);
    free(propDate);
    free(propText);
    free(propTime);

    return returnString;
}

DateTime* JSONtoDT(const char* str) {
    if (str == NULL) {
        return NULL;
    }

    if (strlen(str) <= 0) {
        return NULL;
    }

    char * tempStr = malloc(sizeof(char) * strlen(str) + 1);
    DateTime * returnProp = malloc(sizeof(DateTime) + sizeof(char) * 200);
    returnProp->isText = false;
    returnProp->UTC = false;
    strcpy(returnProp->date, "");
    strcpy(returnProp->text, "");
    strcpy(returnProp->time, "");

    strcpy(tempStr, str);

    int stringLen = strlen(tempStr);

    for (int i = 0; i < stringLen; i++) {
        // isText
        if ((tempStr[i] == '\"') && (tempStr[i+1] == 'i') && (tempStr[i+2] == 's') && (tempStr[i+3] == 'T') && (tempStr[i+4] == 'e') && (tempStr[i+5] == 'x') && (tempStr[i+6] == 't') && (tempStr[i+7] == '\"')&& (tempStr[i+8] == ':')) {
            int start = i + 8;
            if (tempStr[start+1] == 't') {
                returnProp->isText = true;
            }
            else if (tempStr[start+1] == 'f') {
                returnProp->isText = false;                
            }
            else {
                free(tempStr);
                deleteDate(returnProp);
                return NULL;
            }
        }
        // date
        if ((tempStr[i] == '\"') && (tempStr[i+1] == 'd') && (tempStr[i+2] == 'a') && (tempStr[i+3] == 't') && (tempStr[i+4] == 'e') && (tempStr[i+5] == '\"') && (tempStr[i+6] == ':')) {
            int start = i + 7;
            int end = 0;
             for (int j = start+1; j < stringLen; j++) {
                if (tempStr[j] == '\"') {
                    end = j;
                    break;
                }
            }
            if (end == start + 1) {
                strcpy(returnProp->date, "");
            } else {
                char * temp = stringSlicer(&tempStr, start+1, end);
                strcpy(returnProp->date, temp); 
                free(temp);            
            }
        }
        // time
        if ((tempStr[i] == '\"') && (tempStr[i+1] == 't') && (tempStr[i+2] == 'i') && (tempStr[i+3] == 'm') && (tempStr[i+4] == 'e') && (tempStr[i+5] == '\"') && (tempStr[i+6] == ':')) {
            int start = i + 7;
            int end = 0;
             for (int j = start+1; j < stringLen; j++) {
                if (tempStr[j] == '\"') {
                    end = j;
                    break;
                }
            }
            if (end == start + 1) {
                strcpy(returnProp->time, "");
            } else {
                char * temp = stringSlicer(&tempStr, start+1, end);
                strcpy(returnProp->time, temp); 
                free(temp);           
            }
        }
        // text
        if ((tempStr[i] == '\"') && (tempStr[i+1] == 't') && (tempStr[i+2] == 'e') && (tempStr[i+3] == 'x') && (tempStr[i+4] == 't') && (tempStr[i+5] == '\"') && (tempStr[i+6] == ':')) {
            int start = i + 7;
            int end = 0;
            for (int j = start+1; j < stringLen; j++) {
                if (tempStr[j] == '\"') {
                    end = j;
                    break;
                }
            }
            if (end == start + 1) {
                strcpy(returnProp->text, "");
            } else {
                char * temp = stringSlicer(&tempStr, start+1, end);
                strcpy(returnProp->text, temp); 
                free(temp);           
            }
        }
        // isUTC
        if ((tempStr[i] == '\"') && (tempStr[i+1] == 'i') && (tempStr[i+2] == 's') && (tempStr[i+3] == 'U') && (tempStr[i+4] == 'T') && (tempStr[i+5] == 'C') && (tempStr[i+6] == '\"')&& (tempStr[i+7] == ':')) {
            int start = i + 7;
            if (tempStr[start+1] == 't') {
                returnProp->UTC = true;
            }
            else if (tempStr[start+1] == 'f') {
                returnProp->UTC = false;              
            }
            else {
                free(tempStr);
                deleteDate(returnProp);
                return NULL;
            }
        }
    }

    if ((returnProp->isText == false) && (returnProp->UTC == false) && (strlen(returnProp->text) <= 0) && (strlen(returnProp->date) <= 0) && (strlen(returnProp->time) <= 0)) {
        free(tempStr);
        deleteDate(returnProp);
        return NULL;
    }

    VCardErrorCode error = datetimeCheck(returnProp);
    if (error != OK) {
        free(tempStr);
        deleteDate(returnProp);
        return NULL;       
    }
    
    free(tempStr);
    return returnProp;
}

Card* JSONtoCard(const char* str) {
    if (str == NULL) {
        return NULL;
    }

    char * tempStr = malloc(sizeof(char) * strlen(str) + 1);
    Property * returnProp = malloc(sizeof(Property));
    strcpy(tempStr, str);

    int stringLen = strlen(tempStr);
    char * valuesString = NULL;
    char * fnTag = NULL;

    for (int i = 0; i < stringLen; i++) {
        // isText
        if ((tempStr[i] == '\"') && (tempStr[i+1] == 'F') && (tempStr[i+2] == 'N') && (tempStr[i+3] == '\"') && (tempStr[i+4] == ':')) {
            int start = i + 5;
            int end = 0;
            for (int j = start+1; j < stringLen; j++) {
                if (tempStr[j] == '\"') {
                    end = j;
                    break;
                }
            }
            fnTag = malloc(sizeof(char) * 3);
            strcpy(fnTag, "FN");
            returnProp->name = fnTag;

            valuesString = stringSlicer(&tempStr, start+1, end);
        }
    }

    if ((valuesString == NULL) || (fnTag == NULL)) {
        free(tempStr);
        deleteProperty(returnProp);
        return NULL;
    }
    free(tempStr);
    Card * card = malloc(sizeof(Card));
    card->fn = malloc(sizeof(Property));
    card->fn->parameters = initializeList(&printParameter, &deleteParameter, &compareParameters);
    card->fn->values = initializeList(&printValue, &deleteValue, &compareValues);
    insertBack(card->fn->values, (void*)valuesString);
    card->fn->name = fnTag;
    // ===> THIS WAS MALLOC'ed TO FULFILL THE REQUIRMENT OF NOT BEING NULL
    card->fn->group = malloc(sizeof(char) * 1);
    strcpy(card->fn->group, "");


    card->optionalProperties = initializeList(&printProperty, &deleteProperty, &compareProperties);
    card->birthday = NULL;
    card->anniversary = NULL;

    return card;
}

void addProperty(Card* card, const Property* toBeAdded) {
    if (card == NULL) {
        return;
    }
    if (toBeAdded == NULL) {
        return;
    }
    insertBack(card->optionalProperties, (void*)toBeAdded);
    return;
}



