#include "../include/textutils.hpp"

void textCtor(text_t * text, FILE* const source)
{
    readInBuf(text, source);
    loadWords(text);
}

void readInBuf(text_t* text, FILE* source)
{
    text->nChar = fileSize(source);
    text->buf   = (char *) aligned_alloc(32, (text->nChar + 1)*sizeof(char));
    
    fread(text->buf, sizeof(char), text->nChar, source);

}

size_t fileSize(FILE* file)
{
	fseek(file, 0, SEEK_END);
    size_t pos = (size_t) ftell(file);
    fseek(file, 0, SEEK_SET);

    return pos;
}

void loadWords(text_t* text)
{
    size_t   word       = 0;
    size_t   word_start = 0;

    text->word_cnt = 1;
    
    text->words = 
    (word_t*) calloc((text->nChar / 4)  + 1, sizeof(word_t));
    
    int not_alnum = 0;

    for (size_t index = 1; index < text->nChar; index++)
    {
        int check_symb = isalnum(*(text->buf + index));


        if (!check_symb && not_alnum == 0)
        {
            text->word_cnt++;

            not_alnum = 1;

            (*(text->buf + index)= '\0');

            (text->words + word)->word_start = text->buf + word_start;
            ++word;
        }
        else if (check_symb && not_alnum == 1)
        {
            word_start = index;
            not_alnum  = 0;
        }
        

        if (word_start > text->nChar)
            break;
    }
}
     
void textDtor(text_t* text)
{
    free(text->buf); 
    free(text->words);
}

