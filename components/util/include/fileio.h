#pragma once

#include "list.h"
#include "cJSON.h"

/**
 * @brief Get the Files In Dir object
 * 
 * @param list 
 * @param directory 
 * @param ext 
 */
void getFilesInDir(list_t *list, const char *directory, const char *ext);

/**
 * @brief 
 * 
 * @param fileName 
 * @return cJSON* 
 */
cJSON* readJSONFileAsCJSON(const char *fileName);

/**
 * @brief 
 * 
 * @param fileName 
 * @param data 
 */
void writeJSONFile(const char *fileName, const char* data);

/**
 * @brief 
 * 
 * @param file 
 * @return int 
 */
int fileExists(const char *file);

/**
 * @brief Get the File Size object
 * 
 * @param file 
 * @return int 
 */
int getFileSize(const char *file);

/**
 * @brief 
 * 
 * @param data 
 */
void parseJSONAudioTags(cJSON* data);

/**
 * @brief 
 * 
 * @param file      path to file
 * @return int      return 1 if successful, 0 otherwise
 */
int deleteFile(const char* file);

/**
 * @brief Prints internal/external available heap
 * 
 * @param c 
 */
void printHeapInfo(const char* c);

/**
 * @brief Checks structure of SD card, data folders and cfg files
 * 
 */
void checkSDStructure();

/**
 * @brief Repairs config json file and erases file in doubt if it exists
 * 
 * @param id -> voice id
 */
void repairAudioFileAssigment(int id);