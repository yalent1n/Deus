#pragma once
#include <Engine.Base/Bundle.h>
#include <Engine.Base/Buffer/SharedBuffer.h>
#include <Engine.Core/diag/ExecutionContext.h>

namespace rw
{
enum lexer_tok_type : u8
{
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    TOKEN_COMMENT,

    TOKEN_ASSIGN,
    TOKEN_COLON,
    TOKEN_SEMICOLON,
    
    TOKEN_STRING,
    TOKEN_NUMBER,
    
    TOKEN_BEG_CURL,
    TOKEN_END_CURL,
    
    TOKEN_BEG_SQRE,
    TOKEN_END_SQRE,
    
    TOKEN_BEG_PARN,
    TOKEN_END_PARN,
    
    TOKEN_DOT,
    TOKEN_SMALLER,
    TOKEN_GREATER,

    TOKEN_SLASH,
    
    TOKEN_EOF
};

const char* tokenTypeToString(lexer_tok_type typ)
{
    switch(typ)
    {
        case TOKEN_IDENTIFIER: return "TOKEN_IDENTIFIER";
        case TOKEN_KEYWORD: return "TOKEN_KEYWORD";
        case TOKEN_COMMENT: return "TOKEN_COMMENT";
        case TOKEN_ASSIGN: return "TOKEN_ASSIGN";
        case TOKEN_COLON: return "TOKEN_COLON";
        case TOKEN_SEMICOLON: return "TOKEN_SEMICOLON";
        case TOKEN_STRING: return "TOKEN_STRING";
        case TOKEN_NUMBER: return "TOKEN_NUMBER";
        case TOKEN_BEG_CURL: return "TOKEN_BEG_CURL";
        case TOKEN_END_CURL: return "TOKEN_END_CURL";
        case TOKEN_BEG_SQRE: return "TOKEN_BEG_SQRE";
        case TOKEN_END_SQRE: return "TOKEN_END_SQRE";
        case TOKEN_BEG_PARN: return "TOKEN_BEG_PARN";
        case TOKEN_END_PARN: return "TOKEN_END_PARN";
        case TOKEN_DOT: return "TOKEN_DOT";
        case TOKEN_SMALLER: return "TOKEN_SMALLER";
        case TOKEN_GREATER: return "TOKEN_GREATER";
        case TOKEN_SLASH: return "TOKEN_SLASH";
        case TOKEN_EOF: return "TOKEN_EOF";
    }
}

struct lexer_st
{
    const char* text;
    const char* cursor;
    const char* end;
};

struct lexer_tok
{
    const char* token;
    u32 extra;
    lexer_tok_type type;
};

static void lex_init(lexer_st* l, const char* parse_str)
{
    l->text = parse_str;
    l->cursor = parse_str;
    l->end = parse_str + rw::strlen(parse_str);
}

#define lex_peek(l) (*(l)->cursor)
#define lex_read(l) (*(l)->cursor++)

#define lex_deadvance(l) (--(l)->cursor)
#define lex_advance(l) ((l)->cursor++)
#define lex_advance_by(l, n) ((l)->cursor += (n))

#define lex_match_one_char(t, ch, s, tok) case ch: (t)->type = tok; (t)->token = s; return

#define lex_finalize_token(l, typ) \
do { \
    token_end = (l)->cursor; \
    char* val = new char[token_end - token_begin]; \
    memcpy(val, token_begin, size_t(token_end - token_begin)); \
    tok->type = typ; \
    tok->token = val; \
} while(false)

static void lex_match_string_token(lexer_st* l, lexer_tok* tok)
{   
    char term = l->cursor[-1];
    const char* token_begin = l->cursor;
    const char* token_end = nullptr;
    
    char c = lex_peek(l);
    while(c != term && c != '\0')
    {
        if(c == '\\')
        {
            if(lex_peek(l) == 'u')
            {
                lex_advance_by(l, 5);
            }
            else
                lex_advance_by(l, 2);
        }

        c = lex_read(l);
    }

    token_end = lex_deadvance(l);
    lex_finalize_token(l, TOKEN_STRING);
    lex_advance(l);
    tok->extra = 0;
}

#define IS_DEC 0x001
#define IS_HEX 0x002
#define IS_FLT 0x004
#define IS_NEG 0x008

static void lex_match_number_token(lexer_st* l, lexer_tok* tok)
{
    u32 tok_flags = 0;

    const char* token_begin = lex_deadvance(l);
    const char* token_end = nullptr;

    if(lex_peek(l) == '-')
    {
        tok_flags |= IS_NEG;
        lex_advance(l);
    }

    // possibilities: 1.0, 1., .23, 0.31f, 0xffdada, 0b
    char c = lex_read(l);

    if(c == '.')
        tok_flags |= IS_FLT;
    else if(rw::isalnum(c))
        tok_flags |= IS_DEC;

    if(lex_peek(l) == '.' && tok_flags & IS_DEC)
    {
        lex_advance(l);
        tok_flags = (tok_flags & ~IS_DEC) | IS_FLT;
    }

    c = lex_peek(l);
    if(c == 'x' || c == 'X')
    {
        lex_advance(l);
        tok_flags |= IS_HEX;
    }

    tok->extra = tok_flags;    
    
    if(tok_flags & IS_HEX)
    {
        static const char hex_chars[] = "0123456789abcdefABCDEF";
        while(indexOf(lex_peek(l), hex_chars) != -1)
            lex_advance(l);
    }
    else if(tok_flags & IS_DEC)
    {
        while(rw::isalnum(lex_peek(l)))
            lex_advance(l);
    }
    else if(tok_flags & IS_FLT)
    {
        static const char flt_chars[] = "0123456789dDeEfF.+-";
        while(indexOf(lex_peek(l), flt_chars) != -1)
            lex_advance(l);
    }

    // only fixate in the end, token may not be a number
    lex_finalize_token(l, TOKEN_NUMBER);
}

static bool lex_match_comment_token(lexer_st* l, lexer_tok* tok)
{
    tok->extra = l->cursor[1];
    bool is_onelined = char(tok->extra) == '/'; // allow other type of inline comments (like /$ $/ inlines into header)

    // TODO: decide if we wan't value for comment tokens
    if(is_onelined)
    {
        lex_advance(l);

        while(lex_peek(l) != '\r' && lex_peek(l) != '\n')
            lex_advance(l);

        if(lex_peek(l) == '\r')
            lex_advance(l);

        lex_advance(l);
        return true;
    }

    char term = lex_peek(l);
    
    if(term != '*')
        return false;
    
    lex_advance(l);

    while(lex_peek(l) != term || l->cursor[1] != '/')
        lex_advance(l);

    // right now, don't care bout terminator char
    lex_advance_by(l, 2);

    tok->token = nullptr;
    tok->type = TOKEN_COMMENT;
    return true;
}

static constexpr const char* s_keywords[] = {
    // declarators
    "namespace",
    
    "class",
    "struct",
    "enum",

    // qualifiers
    "static",
    "abstract",
    "meta",
    "public",
    "private"
};
static constexpr int keywords_size = sizeof_array(s_keywords);

static void lex_detect_keyword(lexer_tok* tok)
{
    for(int i = 0; i < keywords_size; i++)
    {
        if(equalsIgnoreCase(s_keywords[i], tok->token))
        {
            tok->type = TOKEN_KEYWORD;
            tok->extra = u32(i);
            break;
        }
    }
}

static void lex_next_token(lexer_st* l, lexer_tok* tok)
{
    if(l->cursor >= l->end)
    {
        if(tok)
            tok->type = TOKEN_EOF;

        return;
    }

    char c;

    {
        c = lex_peek(l);
        if(rw::isspace(c))
        {
            while(rw::isspace(c) && c != '\0')
                c = lex_read(l);
    
            l->cursor--;
        }
    }

    c = lex_read(l);

    switch(c)
    {
        lex_match_one_char(tok, ':', ":", TOKEN_COLON);
        lex_match_one_char(tok, ';', ";", TOKEN_SEMICOLON);
        lex_match_one_char(tok, '=', "=", TOKEN_ASSIGN);

        lex_match_one_char(tok, '{', "{", TOKEN_BEG_CURL);
        lex_match_one_char(tok, '}', "}", TOKEN_END_CURL);
        
        lex_match_one_char(tok, '[', "[", TOKEN_BEG_SQRE);
        lex_match_one_char(tok, ']', "]", TOKEN_END_SQRE);

        lex_match_one_char(tok, '(', "(", TOKEN_BEG_PARN);
        lex_match_one_char(tok, ')', ")", TOKEN_END_PARN);
        
        lex_match_one_char(tok, '.', ".", TOKEN_DOT);
        lex_match_one_char(tok, '<', "<", TOKEN_SMALLER);
        lex_match_one_char(tok, '>', ">", TOKEN_GREATER);

        // parse char and strings
        case '\'':
        case '\"':
            lex_match_string_token(l, tok);
            return;

        default:
            break;
    }

    // parse comments
    if(c == '/')
    {
        if(!lex_match_comment_token(l, tok))
        {
            tok->type = TOKEN_SLASH;
            tok->token = "/";
        }

        return;
    }

    // TODO: have config if we want to parse numbers
    // parse numbers
    // if(rw::isalnum(c) || (c == '-' && rw::isalnum(lex_peek(l))))
    // {
    //     lex_match_number_token(l, tok);
    //     return;
    // }

    const char* pident = l->cursor - 1;
    // allowed chars: a-z, A-Z, 0-9, _,
    StringBuilder<> token_val;
    token_val << c;

    {
        c = lex_read(l);

        while(rw::isalchar(c) || rw::isalnum(c) || c == '_')
        {
            token_val << c;
            c = lex_read(l);
        }

        l->cursor--;
    }

    tok->extra = u32(pident - l->text);
    tok->token = token_val.detach();
    tok->type = TOKEN_IDENTIFIER;

    lex_detect_keyword(tok);
}

static void lex_close(lexer_st* l)
{
    l->cursor = nullptr;
    l->end = nullptr;
}

struct par_expression_op
{
    int lbp; // Left Binding Power
    int rbp; // Right Binding Power
};

static inline par_expression_op sm_ExpressionOperators[TOKEN_EOF] = {
    {}, // TOKEN_IDENTIFIER
    {}, // TOKEN_KEYWORD
    {0, 0}, // TOKEN_COMMENT
    {}, // TOKEN_ASSIGN
    {}, // TOKEN_COLON
    {}, // TOKEN_SEMICOLON
    {}, // TOKEN_STRING
    {}, // TOKEN_NUMBER
    {}, // TOKEN_BEG_CURL
    {}, // TOKEN_END_CURL
    {}, // TOKEN_BEG_SQRE
    {}, // TOKEN_END_SQRE
    {}, // TOKEN_BEG_PARN
    {}, // TOKEN_END_PARN
    {}, // TOKEN_DOT
    {}, // TOKEN_SMALLER
    {}, // TOKEN_GREATER
};

struct parser_st
{
    lexer_tok* tokens;
    lexer_tok* end;
    lexer_tok* cursor;
};

static void par_init(parser_st* par, lexer_tok* tok, size_t toksz)
{
    if(!par)
        return;

    par->tokens = par->cursor = tok;
    par->end = tok + toksz;
}

static lexer_tok* par_consume(parser_st* par)
{
    if(par->cursor >= par->end || par->cursor < par->tokens)
        return nullptr;

    return ++par->cursor;
}

static lexer_tok* par_peek(parser_st* par)
{
    if(par->cursor >= par->end || par->cursor < par->tokens)
        return nullptr;

    return par->cursor;
}

struct parser_expression
{
};

static parser_expression* par_parse_prefix(parser_st* par)
{

}

static int par_parse_infix(parser_st* par, parser_expression* expr)
{
    
}

static void par_next_expression(parser_st* par, parser_expression* expr, int rbp)
{
    lexer_tok_type ltype = par_peek(par)->type;
    parser_expression* left = par_parse_prefix(par);

    while(sm_ExpressionOperators[ltype].lbp < rbp)
    {

    }
}

// int run_lexer()
// {
//     ScopedRef ddfData = DEVICE.shareFile("app:/lexertest.txt");

//     lexer_st lex;
//     lex_init(&lex, (const char*) ddfData->bytes());

//     eastl::vector<lexer_tok> tokens;

//     lexer_tok token;
//     for(;;)
//     {
//         lex_next_token(&lex, &token);
//         if(token.type == TOKEN_EOF)
//             break;

//         tokens.push_back(token);
//     }
    
//     lex_close(&lex);

//     parser_st par;
//     par_init(&par, tokens.begin(), tokens.size());

//     parser_expression expr;
//     for(;;)
//     {
//         par_next_expression(&par, &expr, 0);
//         if(expr.type == EXPR_EOF)
//             break;

//         par_debug_expression(&expr);
//     }

//     return true;
// }
}