#include <assert.h>
#include <string.h>

//---------------------------------------------------------

#include "word_def.h"

//=========================================================

int def_word_ctor_def(struct Def_word* def_word)
{
    assert(def_word);

    def_word->data = NULL;
    def_word->len  = 0;

    return 0;
};

//---------------------------------------------------------

int def_word_ctor_copy(struct Def_word* def_word, struct Def_word* src)
{
    assert(def_word);
    assert(src);

    if (src->data = NULL)
    {
        def_word->data = NULL;
        def_word->len  = 0;

        return 0; 
    }

    def_word->data = (char*) calloc(src->len, sizeof(char));
    if (def_word->data == NULL)
        return -1;

    def_word->len = src->len;
    strncpy(def_word->data, src->data, def_word->len);

    return 0;
};

//---------------------------------------------------------

int def_word_copy(struct Def_word* dst, struct Def_word* src)
{
    assert(dst);
    assert(src);

    if (dst->len >= src->len)
    {
        strncpy(dst->data, src->data, src->len);
        dst->len = src->len;
    }
    else 
    {
        if (dst->data != NULL)
        {
            free(dst->data);
        }

        dst->data = (char*) calloc(src->len, sizeof(char));
        if (dst->data == NULL)
            return -1;

        strncpy(dst->data, src->data, src->len);
        dst->len = src->len;
    }

    return 0;
};

//---------------------------------------------------------

int def_word_dtor(struct Def_word* def_word)
{
    assert(def_word);

    if (def_word->data != NULL)
    {
        free(def_word->data);
        def_word->data = NULL;
    }

    def_word->len = 0;
    return 0;
};

//---------------------------------------------------------

int def_word_show(struct Def_word* def_word, FILE* out_fd)
{
    assert(def_word);
    assert(out_fd);

    return fprintf(out_fd, "%s", def_word->data);
}

//---------------------------------------------------------

int def_word_is_empty(struct Def_word* def_word)
{
    assert(def_word);
    
    return (def_word->data == NULL);
}
