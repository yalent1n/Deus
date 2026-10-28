#include <Engine.Network/http/router.h>
#include <Engine.Core/Lexer.h>

namespace rw
{
bool HTTPPartUri(const char* path, eastl::vector<http_path_part>& parts)
{
    lexer_st lex;
    lex_init(&lex, path);

    eastl::vector<lexer_tok> tokens;

    lexer_tok token;
    for(;;)
    {
        lex_next_token(&lex, &token);
        if(token.type == TOKEN_EOF)
            break;

        tokens.push_back(token);
    }

    lexer_tok* tokcsr = tokens.begin();
    while(tokcsr < tokens.end())
    {
        if(tokcsr->type != TOKEN_SLASH)
            return false;

        tokcsr++;
        if(tokcsr->type == TOKEN_BEG_PARN) // capture/remaining
        {
            tokcsr++;
            const char* capture_name = tokcsr->token;

            bool is_wildcard = capture_name[0] == '*';
            if(is_wildcard)
                capture_name++;

            parts.push_back() = {capture_name, (is_wildcard ? http_path_part::CAPTURE_WILDCARD : http_path_part::CAPTURE), s16(tokcsr->extra)};

            tokcsr++;
            if(tokcsr->type != TOKEN_END_PARN)
                return false;

            tokcsr++;
        }
        else
        {
            const char* label = tokcsr->token;
            parts.push_back() = {label, http_path_part::LABEL, s16(tokcsr->extra)};
            tokcsr++;
        }
    }

    lex_close(&lex);
    return true;
}

HttpRouter::HttpRouter(const char* route)
    : m_URI(route)
{
    HTTPPartUri(route, m_Parts);
}

bool HttpRouter::doesURIMatch(const char* uri)
{
    eastl::vector<http_path_part> passed_parts;
    HTTPPartUri(uri, passed_parts);

    if(m_Parts.back().Type != http_path_part::CAPTURE_WILDCARD && passed_parts.size() != m_Parts.size())
    {
        return false;
    }

    u32 parts_count = m_Parts.size();
    for(u32 i = 0; i < parts_count; i++)
    {
        if(i >= passed_parts.size())
            return false;

        const http_path_part& pass_part = passed_parts[i];
        const http_path_part& template_part = m_Parts[i];

        // We can leave out remaining from the check because it's usually passed down to another
        // handler thingy.
        if(template_part.Type == http_path_part::LABEL)
        {
            bool Equals = equalsIgnoreCase(pass_part.Text, template_part.Text);
            if(!Equals)
                return false;
        }
    }

    return true;
}

bool HttpRouter::doesURIMatchAndCapture(const char* uri, eastl::vector<http_path_param>& captures)
{
    eastl::vector<http_path_part> passed_parts;
    HTTPPartUri(uri, passed_parts);

    if(m_Parts.back().Type != http_path_part::CAPTURE_WILDCARD && passed_parts.size() != m_Parts.size())
    {
        return false;
    }

    u32 parts_count = m_Parts.size();
    for(u32 i = 0; i < parts_count; i++)
    {
        if(i >= passed_parts.size())
            return false;

        const http_path_part& pass_part = passed_parts[i];
        const http_path_part& template_part = m_Parts[i];

        // We can leave out remaining from the check because it's usually passed down to another
        // handler thingy.
        if(template_part.Type == http_path_part::LABEL)
        {
            bool Equals = equalsIgnoreCase(pass_part.Text, template_part.Text);
            if(!Equals)
                return false;
        }

        if(template_part.Type == http_path_part::CAPTURE || template_part.Type == http_path_part::CAPTURE_WILDCARD)
        {
            bool is_wildcard = template_part.Type == http_path_part::CAPTURE_WILDCARD;
            const char* capture = pass_part.Text;
            
            if(is_wildcard)
            {
                const char* rest_str = uri + pass_part.Offset;
                capture = safedup(rest_str);
            }

            captures.push_back({template_part.Text, capture});
        }
    }

    return true;
}
}