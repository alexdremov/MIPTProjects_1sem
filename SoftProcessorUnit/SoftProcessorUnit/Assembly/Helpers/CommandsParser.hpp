//
//  CommandsParser.hpp
//  SPUAsm
//
//  Created by Александр Дремов on 12.10.2020.
//

#ifndef CommandsParser_hpp
#define CommandsParser_hpp

#include <stdio.h>
#include "CommandsDTypes.hpp"
#include "AssemblyHelpers.hpp"
#include "Syntax.hpp"

char* getSourceFileData(FILE* inputFile, size_t* length);

void preprocessSource(char* code, size_t* length);

int codeBlockEmpty(char* codeBlock);

CommandParseResult parseCommand(AssemblyParams* compileParams, const struct SyntaxMapping* mapping, BinaryFile* binary, char* codeBlock);

CommandParseResult parseCode(AssemblyParams* compileParams, const struct SyntaxMapping* mapping, BinaryFile* binary, char* code, size_t length);

const struct SyntaxEntity* fetchCommand(const struct SyntaxMapping* mapping, char* codeBlock);

const char** getArgList(char* codeBlock, int* argc);

int isValidArgumentsNumber(const struct SyntaxEntity* mapping, int argc);

int registerNoFromName(char* name);

const char* registerNameFromNo(int no);

void codeEstimations(BinaryFile* binary, char* code);

#endif /* CommandsParser_hpp */