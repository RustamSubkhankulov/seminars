#include <assert.h>
#include <string.h>
#include <ctype.h>

//---------------------------------------------------------

#include "word_clang.h"

//=========================================================

// here clang means name that can be given to function in ANSI C

static const int Max_len_clang_word = 31;

//=========================================================

int clang_word_read(struct Def_word* def_word, char* input)
{
    assert(def_word);
    assert(input);

    struct Clang_word* clang_word = (struct Clang_word*) def_word;

    while (*input == '\n' || *input == '\t' || *input == ' ')
        continue;

    char* word_start = input;
    size_t len = 0;

    while (*input != '\0' 
       && (len < Max_len_clang_word)
       && isalnum(*input))
    {
        input++;
        len++;
    }

    if (len == 0)
        return 0;

    char* word_storage = (char*) calloc(len + 1, sizeof(char));
    if (word_storage == NULL)
        return -1;

    memcpy(word_storage, word_start, len);
    *(word_start + len) = '\0';

    clang_word->def_word.data = word_storage;
    clang_word->def_word.len = len; 

    return 0;
}