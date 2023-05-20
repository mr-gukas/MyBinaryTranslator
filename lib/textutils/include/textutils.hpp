#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

struct word_t 
{
    char*  word_start = NULL;
};

struct text_t
{
    char *  buf      = NULL;
    size_t  nChar    = 0;
    word_t* words    = NULL;
    size_t  word_cnt = 0;
};

void textCtor(text_t* text, FILE* const source);
size_t fileSize(FILE* const file);
void countInText(char* buf, char search, size_t* count);
void readInBuf(text_t* text, FILE* source);
void loadWords(text_t* text);
void textDtor(text_t* text); 
