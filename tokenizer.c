#include <Python.h>

#define ARRAY_SIZE_OF(x) (sizeof(x) / sizeof(x[0]))

// Hacky way around the differences between UCS2 and UCS4 systems. Since we can't
// rely on the fact that sizeof(wchar_t) == sizeof(Py_UNICODE), we do everything
// in the native Py_UNICODE format.
Py_UNICODE sep_chars[] = {(Py_UNICODE)' ', (Py_UNICODE)'\t', (Py_UNICODE)'\n', (Py_UNICODE)'\r', (Py_UNICODE)'*'};
const Py_UNICODE special_token_sequences[3][5] = {
    {3, 'A', '\'', 'A', '\0'},
    {3, 'N', '.', 'N', '\0'},
    {2, '.', 'N', '\0'}
};

size_t pyunicode_slen(const Py_UNICODE* p_str)
{
    const Py_UNICODE* p_end = p_str;
    while(*p_end++);
    return (p_end - p_str - 1);
}

char get_char_type(Py_UNICODE wc)
{
    if (Py_UNICODE_ISALPHA(wc))
        return 'A';
    
    if (Py_UNICODE_ISDIGIT(wc))
        return 'N';
    
    return 'P';
}

int is_valid_token(const Py_UNICODE* str, int start_pos, int end_pos)
{
    if (start_pos == end_pos)
        return 0;
    
    // break out if the word is actually a separator
    if (start_pos + 1 == end_pos)
    {
        Py_UNICODE c = str[start_pos];
        unsigned int i;
        for(i = 0; i < ARRAY_SIZE_OF(sep_chars); ++i)
            if (c == sep_chars[i])
                return 0;
    }
    
    return 1;
}

int skip_over_special_token_sequence(const Py_UNICODE* str, int pos)
{
    // returns the number of characters to skip over
    Py_UNICODE cur_char;
    Py_UNICODE match_char;
    unsigned int idx;
    for( idx = 0; idx < ARRAY_SIZE_OF(special_token_sequences) ; ++idx)
    {
        const Py_UNICODE* sequence = special_token_sequences[idx];
        int offset = 0;
        int matched = 1;
        unsigned int seq_len = sequence[0];
        unsigned int s_idx;
        for( s_idx = 0; s_idx < seq_len; ++s_idx)
        {
            cur_char = str[pos + offset];
            if(!cur_char)
                return 0;
            
            match_char = sequence[s_idx+1];
            if (! ((Py_UNICODE_ISUPPER(match_char) && (get_char_type(cur_char) == match_char)) || (cur_char == match_char)))
            {
                matched = 0;
                break;
            }
            offset += 1;
        }
        if (matched)
            return seq_len;
    }
    return 0;
}

typedef struct _pair
{
    int first;
    int second;
} pair;

void parse_str(const Py_UNICODE* str, pair** pp_token_locs, size_t* p_token_count)
{
    int data_len = pyunicode_slen(str);
    int pos = 0;
    int word_start_pos = 0;
 
    size_t data_size = data_len * sizeof(pair);
    pair* p_token_locs = (pair*)malloc(data_size);
    *pp_token_locs = p_token_locs;
    memset(*pp_token_locs, 0, data_size);
    size_t cur_tok_idx = 0;
 
    char word_type = get_char_type(str[word_start_pos]);

//    printf ("data_len: %d data_size: %d\n", data_len, data_size);
    while (pos < data_len)
    {
//        printf("pos: %d word_start_pos: %d data_len: %d cur_tok_idx: %d\n", pos, word_start_pos, data_len, cur_tok_idx);
        Py_UNICODE cur_char = str[pos];
        char cur_char_type = get_char_type(cur_char);
        
        if ((cur_char_type != word_type) || (word_type == 'P'))
        {
            if (is_valid_token(str, word_start_pos, pos))
            {
                p_token_locs[cur_tok_idx].first = word_start_pos;
                p_token_locs[cur_tok_idx++].second = pos;
            }
            
            word_start_pos = pos;
            word_type = get_char_type(str[word_start_pos]);
        }
        
        int skip_chars = skip_over_special_token_sequence(str, pos);
        if (skip_chars > 0)
        {
            pos += skip_chars;
            word_type = get_char_type(str[pos-1]);
        }
        else
            pos += 1;
    }
    
    if (is_valid_token(str, word_start_pos, pos))
    {
        p_token_locs[cur_tok_idx].first = word_start_pos;
        p_token_locs[cur_tok_idx++].second = pos;
    }
    
    *p_token_count = cur_tok_idx;
}

static PyObject* tokenize(PyObject* self, PyObject* args)
{
    const Py_UNICODE* str;
    int return_token_pos = 0;

    if (!PyArg_ParseTuple(args, "u|i", &str, &return_token_pos))
        return NULL;

    pair* p_token_locs = 0;
    size_t token_count = 0;
    parse_str(str, &p_token_locs, &token_count);
    
    PyObject* parsed_tokens = PyTuple_New(token_count);
    if (!parsed_tokens)
    {
        free(p_token_locs);
        return NULL;
    }
    
    size_t idx = 0;
    for (idx = 0; idx < token_count; ++idx)
    {
        int start_loc = p_token_locs[idx].first;
        PyObject* token = PyUnicode_FromUnicode(&str[start_loc], p_token_locs[idx].second - start_loc);
        if (!token)
            break;
        
        if (return_token_pos)
        {
            PyObject* token_data = PyTuple_New(2);
            PyTuple_SetItem(token_data, 0, token);
            PyTuple_SetItem(token_data, 1, PyInt_FromLong(start_loc));
            PyTuple_SetItem(parsed_tokens, idx, token_data);
        }
        else
            PyTuple_SetItem(parsed_tokens, idx, token);
    }
    
    free(p_token_locs);
    
    if(idx != token_count)
    {
        Py_DECREF(parsed_tokens);
        return NULL;
    }
    
    return parsed_tokens;
}

static PyMethodDef TokenizeMethods[] =
{
     {"tokenize", tokenize, METH_VARARGS, "tokenize a unicode string."},
     {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initTokenizer(void)
{
    (void) Py_InitModule("Tokenizer", TokenizeMethods);
}
