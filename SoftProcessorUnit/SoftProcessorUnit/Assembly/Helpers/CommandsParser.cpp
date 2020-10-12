//
//  CommandsParser.cpp
//  SPUAsm
//
//  Created by Александр Дремов on 12.10.2020.
//

#include "CommandsParser.hpp"
#include "AssemblyHelpers.hpp"
#include "Syntax.hpp"
#include <string.h>
#include <stdio.h>
#include <ctype.h>


char* getSourceFileData(FILE* inputFile, size_t* length) {
    fseek(inputFile, 0, SEEK_END);
    *length = (size_t) ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);
    
    char *buffer = (char*)calloc(*length + 2, sizeof(char));
    if (buffer) {
        fread(buffer, 1, *length, inputFile);
        buffer[*length] = '\0';
    } else {
        return NULL;
    }
    
    return buffer;
}

void preprocessSource(char* code, size_t* length) {
    char* commentPos = strchr(code, ';');
    while (commentPos != NULL) {
        *commentPos = '\0';
        commentPos = strchr(commentPos + 1, ';');
    }
    
    for (int i = 0; i + 1 < *length; ) {
        if(code[i] == ' ' && (code[i+1] == ' ' || code[i-1] == ' ')) {
            for(int j = i; j < *length; j++) {
                code[j]=code[j + 1];
            }
            (*length)--;
        } else {
            i++;
        }
    }
}

const SyntaxEntity* fetchCommand(const SyntaxMapping* mapping, char* codeBlock) {
    char* command = (char*) calloc(strlen(codeBlock) + 1, sizeof(char));
    strcpy(command, codeBlock);
    char* firstWhitespace = strchr(command, ' ');
    if (firstWhitespace != NULL){
        *firstWhitespace = '\0';
    } else {
        firstWhitespace = strchr(command, '\n');
        if (firstWhitespace != NULL){
            *firstWhitespace = '\0';
        }
    }
    
    const SyntaxEntity* foundCommand = getSyntaxEntityByName(mapping, (const char*)command);
    
    free(command);
    return foundCommand;
}

int isValidArgumentsNumber(const SyntaxEntity* mapping, char* codeBlock, int* hasArguments) {
    char* firstWhitespace = strchr(codeBlock, ' ');
    if(firstWhitespace == NULL && strlen(mapping->format) == 0)
        return 1;
    
    int argumentsAvailable = 0;
    if (firstWhitespace != NULL) {
        firstWhitespace = strchr(firstWhitespace, ' ');
        while (firstWhitespace != NULL) {
            if (!codeBlockEmpty(firstWhitespace))
                argumentsAvailable++;
            firstWhitespace = strchr(firstWhitespace + 1, ' ');
        }
    }
    
    const char* formatPtr = mapping->format;
    
    int maxPossible = 0;
    int argumentsTotal = argumentsAvailable;
    *hasArguments = argumentsTotal;
    while (*formatPtr != '\0') {
        if (*formatPtr == '*' && argumentsAvailable <= 0)
            return 0;
        argumentsAvailable--;
        formatPtr++;
        maxPossible++;
    }
    
    if (argumentsTotal > maxPossible)
        return 0;
    
    return 1;
}

CommandParseResult parseCommand(AssemblyParams* compileParams, const SyntaxMapping* mapping, BinaryFile* binary, char* codeBlock) {
    char* newlinePos = strchr(codeBlock, '\n');
    if (newlinePos != NULL){
        *newlinePos = '\0';
    }
    
    const SyntaxEntity* foundEntity = fetchCommand(mapping, codeBlock);
    if (foundEntity == NULL){
        if (compileParams->verbose) {
            printf("assembly: unknown instruction '%s' found\n", codeBlock);
        }
        fprintf(compileParams->lstFile, "assembly: unknown instruction '%s' found\n", codeBlock);
        return SPU_UNKNOWN_COMMAND;
    }
    
    int hasArguments = 0;
    int validArguments = isValidArgumentsNumber(foundEntity, codeBlock, &hasArguments);
    if (validArguments == 0){
        if (compileParams->verbose) {
            printf("assembly: wrong instruction '%s' found. "
                   "Arguments number is not valid. "
                   "Valid format: '%s'\n", codeBlock, foundEntity->format);
        }
        fprintf(compileParams->lstFile, "assembly: wrong instruction '%s' found. "
               "Arguments number is not valid. "
               "Valid format: '%s'\n", codeBlock, foundEntity->format);
        return SPU_CMD_WRONG_ARGUMENTS;
    }
    

    
    if (newlinePos != NULL){
        *newlinePos = '\n';
    }
    
    return SPU_PARSE_OK;
}

int codeBlockEmpty(char* codeBlock) {
    while (*codeBlock != '\0' && *codeBlock != '\n') {
        if (isprint(*codeBlock) && *codeBlock != ' ')
            return 0;
        codeBlock++;
    }
    return 1;
}

CommandParseResult parseCode(AssemblyParams* compileParams, const SyntaxMapping* mapping, BinaryFile* binary, char* code, size_t length) {
    char* lastBlockPos = code;
    
    size_t instrUct = 1;
    while (lastBlockPos != NULL && lastBlockPos != (char*)NULL + 1) {
        if (!codeBlockEmpty(lastBlockPos)) {
            CommandParseResult res = parseCommand(compileParams, mapping, binary, lastBlockPos);
            if (res != SPU_PARSE_OK){
                if (compileParams->verbose) {
                    printf("assembly: failed to parse instruction no. %zu '%s'\n", instrUct, lastBlockPos);
                }
                fprintf(compileParams->lstFile, "assembly: foiled to parse instruction no. %zu '%s'\n", instrUct, lastBlockPos);
                return res;
            }
            instrUct++;
        }
        lastBlockPos = ((char*)memchr(lastBlockPos, '\n', length - (lastBlockPos - code))) + 1;
    }
    
    
    return SPU_PARSE_OK;
}
