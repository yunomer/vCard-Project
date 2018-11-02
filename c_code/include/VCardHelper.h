#ifndef _CARDHELPER_H
#define _CARDHELPER_H

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "VCardParser.h"
#include "LinkedListAPI.h"

typedef struct validTags {
    int begin;
    int end;
    int version;
    int fn;
    int kind;
    int n;
    int bday;
    int anniversary;
    int gender;
    int prodid;
    int rev;
    int uid;
    int source;
    int xml;
    int nickname;
    int photo;
    int adr;
    int tel;
    int email;
    int impp;
    int lang;
    int tz;
    int geo;
    int title;
    int role;
    int logo;
    int org;
    int member;
    int related;
    int categories;
    int note;
    int sound;
    int clientpidmap;
    int url;
    int key;
    int fburl;
    int calanderuri;
    int caluri;
} Tags;


/** 
 * Function that evaluate if the file extention is corrent or not
 * @param: It will take pointer to file name
 * @return: true (1) if file extention is correct
 *          false (0) if file extention is wrong
 **/
bool validateFileExtention (char * fileName);

/** 
 * Function that will pre-process the file
 * It will unfold the lines and place each line in a malloc'ed array
 * @param: pointer to the file name && double char pointer 
 * @return: -1 if file failed to open or read
 *           0 if no lines could be read or file empty
 *          >0 if file is successful (which will also indicate number of malloc'ed arrays)
 **/
VCardErrorCode preProcessFile (char * fileName, char *** fileArray);

/** 
 * Function that checks to see if the file meets the basic requirments of being a valid vcard file
 * @param: It will take pointer to the fileArray and array size
 * @return: True for contains basic properties, false for if it failed.
 **/
bool initialCheckFile (char *** fileArray, int arraySize);

/** 
 * Function that deletes the file that's saved in an array by preProcessFile
 * @param: It will take pointer to the fileArray and the arraySize 
 * @return: Does not return anything.
 **/
void deleteSavedFile ( char *** fileArray, int arraySize);


/*
 * I got this from StackOverflow:
 * https://stackoverflow.com/questions/10375728/copy-character-from-string-to-another-string-in-c
*/
char * stringSlicer (char ** string, int start, int end);

/** 
 * Function that parses FN property and places it in the Card
 * @param: It will take pointer to a string to parse and the card object to place
 *         the newly created property object in
 * @return: Returns a VCardErrorCode
 **/
VCardErrorCode fnPropertyParser ( char ** string, Card** obj);

/** 
 * Function that parses date and time from a string and places it in the Card
 * @param: It will take pointer to a string to parse and the card object to place
 *         the newly created property object in.
 * @return: Returns a VCardErrorCode
 **/
VCardErrorCode dateTimeParser ( char ** string, DateTime ** obj);

/** 
 * Function that parses Optional property and places it in the Card (Exact same to FN)
 * @param: It will take pointer to a string to parse and the card object to place
 *         the newly created property object in
 * @return: Returns a VCardErrorCode
 **/
VCardErrorCode optionalPropertyParser ( char ** string, Card** obj);

/** 
 * Function that acts link the distribution center, sends the strings to the right parser
 * @param: It will take pointer to the array of strings it needs to distribute, number of lines
 *         in the file and the card object pointer.
 * @return: Returns a VCardErrorCode
 **/
VCardErrorCode processFile(char *** fileArray, int arraySize, Card ** obj);

/** 
 * Function that parses Parameters, it seperated the string it's sent into params
 * @param: It will take pointer to a string to parse and the parameter object to place 
 *         the newly created values in.
 * @return: Returns a VCardErrorCode
 **/
VCardErrorCode paramProcess(char ** returnString, Parameter ** ptr);

/** 
 * Function that's basically a custom toString from linkedList functions. It does a custom toString
 * @param: It will take pointer to the list it properties it needs to convert to string.
 * @return: Pointer to a malloc'ed string
 **/
char* toStringOpProps(List * list);

/** 
 * Function that prints a custom string for datetime object.
 * @param: It will take a pointer to the datetime object and pointer to the string that's either
 *         BDAY or ANNIVERSARY
 * @return: Pointer to a malloc'ed string
 **/
char* printBdayAnni(void* toBePrinted, char * name);

/** 
 * Function that parses a property object making sure it follows the proper format of the Spec
 * @param: It will take pointer to a Property Object
 * @return: Returns a VCardErrorCode
 **/
VCardErrorCode propertyObjCheck(Property * ptr);

/** 
 * Function that parses a DateTime Object making sure it follows the proper format of the Spec
 * @param: It will take pointer to a Property Object
 * @return: Returns a VCardErrorCode
 **/
VCardErrorCode datetimeCheck(DateTime * ptr);

VCardErrorCode propNameChecker (Property * ptr, Tags * tags);

#endif	