#include <Engine.Network/http/base.h>

namespace rw
{
const char* CHttpStatus::toString(HttpStatus e)
{
    if(e == HTTP_CONTINUE) 
        return "Continue";
    else if(e == HTTP_SWITCHING_PROTOCOLS) 
        return "Switching Protocols";
    else if(e == HTTP_PROCESSING) 
        return "Processing";
    else if(e == HTTP_EARLY_HINTS) 
        return "Early Hints";
    else if(e == HTTP_OK) 
        return "OK";
    else if(e == HTTP_CREATED) 
        return "Created";
    else if(e == HTTP_ACCEPTED) 
        return "Accepted";
    else if(e == HTTP_NON_AUTHORITATIVE_INFORMATION) 
        return "Non-Authoritative Information";
    else if(e == HTTP_NO_CONTENT) 
        return "No Content";
    else if(e == HTTP_RESET_CONTENT) 
        return "Reset Content";
    else if(e == HTTP_PARTIAL_CONTENT) 
        return "Partial Content";
    else if(e == HTTP_MULTI_STATUS) 
        return "Multi-Status";
    else if(e == HTTP_ALREADY_REPORTED) 
        return "Already Reported";
    else if(e == HTTP_IM_USED) 
        return "IM Used";
    else if(e == HTTP_MULTIPLE_CHOICES) 
        return "Multiple Choices";
    else if(e == HTTP_MOVED_PERMANENTLY) 
        return "Moved Permanently";
    else if(e == HTTP_FOUND) 
        return "Found";
    else if(e == HTTP_SEE_OTHER) 
        return "See Other";
    else if(e == HTTP_NOT_MODIFIED) 
        return "Not Modified";
    else if(e == HTTP_USE_PROXY) 
        return "Use Proxy";
    else if(e == HTTP_TEMPORARY_REDIRECT) 
        return "Temporary Redirect";
    else if(e == HTTP_PERMANENT_REDIRECT) 
        return "Permanent Redirect";
    else if(e == HTTP_BAD_REQUEST) 
        return "Bad Request";
    else if(e == HTTP_UNAUTHORIZED) 
        return "Unauthorized";
    else if(e == HTTP_PAYMENT_REQUIRED) 
        return "Payment Required";
    else if(e == HTTP_FORBIDDEN) 
        return "Forbidden";
    else if(e == HTTP_NOT_FOUND) 
        return "Not Found";
    else if(e == HTTP_METHOD_NOT_ALLOWED) 
        return "Method Not Allowed";
    else if(e == HTTP_NOT_ACCEPTABLE) 
        return "Not Acceptable";
    else if(e == HTTP_PROXY_AUTH_REQUIRED) 
        return "Proxy Authentication Required";
    else if(e == HTTP_REQUEST_TIMEOUT) 
        return "Request Timeout";
    else if(e == HTTP_CONFLICT) 
        return "Conflict";
    else if(e == HTTP_GONE) 
        return "Gone";
    else if(e == HTTP_LENGTH_REQUIRED) 
        return "Length Required";
    else if(e == HTTP_PRECONDITION_FAILED) 
        return "Precondition Failed";
    else if(e == HTTP_PAYLOAD_TOO_LARGE) 
        return "Payload Too Large";
    else if(e == HTTP_URI_TOO_LONG) 
        return "URI Too Long";
    else if(e == HTTP_UNSUPPORTED_MEDIA_TYPE) 
        return "Unsupported Media Type";
    else if(e == HTTP_RANGE_NOT_SATISFIABLE) 
        return "Range Not Satisfiable";
    else if(e == HTTP_EXPECTATION_FAILED) 
        return "Expectation Failed";
    else if(e == HTTP_IM_A_TEAPOT) 
        return "I'm a teapot";
    else if(e == HTTP_MISDIRECTED_REQUEST) 
        return "Misdirected Request";
    else if(e == HTTP_UNPROCESSABLE_CONTENT) 
        return "Unprocessable Content";
    else if(e == HTTP_LOCKED) 
        return "Locked";
    else if(e == HTTP_FAILED_DEPENDENCY) 
        return "Failed Dependency";
    else if(e == HTTP_TOO_EARLY) 
        return "Too Early";
    else if(e == HTTP_UPGRADE_REQUIRED) 
        return "Upgrade Required";
    else if(e == HTTP_PRECONDITION_REQUIRED) 
        return "Precondition Required";
    else if(e == HTTP_TOO_MANY_REQUESTS) 
        return "Too Many Requests";
    else if(e == HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE) 
        return "Request Header Fields Too Large";
    else if(e == HTTP_UNAVAILABLE_FOR_LEGAL_REASONS) 
        return "Unavailable For Legal Reasons";
    else if(e == HTTP_INTERNAL_SERVER_ERROR) 
        return "Internal Server Error";
    else if(e == HTTP_NOT_IMPLEMENTED) 
        return "Not Implemented";
    else if(e == HTTP_BAD_GATEWAY) 
        return "Bad Gateway";
    else if(e == HTTP_SERVICE_UNAVAILABLE) 
        return "Service Unavailable";
    else if(e == HTTP_GATEWAY_TIMEOUT) 
        return "Gateway Timeout";
    else if(e == HTTP_HTTP_VERSION_NOT_SUPPORTED) 
        return "HTTP Version Not Supported";
    else if(e == HTTP_VARIANT_ALSO_NEGOTIATES) 
        return "Variant Also Negotiates";
    else if(e == HTTP_INSUFFICIENT_STORAGE) 
        return "Insufficient Storage";
    else if(e == HTTP_LOOP_DETECTED) 
        return "Loop Detected";
    else if(e == HTTP_NOT_EXTENDED) 
        return "Not Extended";
    else if(e == HTTP_NETWORK_AUTH_REQUIRED) 
        return "Network Authentication Required";

    return nullptr;
}

HttpStatus CHttpStatus::fromString(const char* s)
{
    if(equalsIgnoreCase("Continue", s))
        return HTTP_CONTINUE;
    if(equalsIgnoreCase("Switching Protocols", s))
        return HTTP_SWITCHING_PROTOCOLS;
    if(equalsIgnoreCase("Processing", s))
        return HTTP_PROCESSING;
    if(equalsIgnoreCase("Early Hints", s))
        return HTTP_EARLY_HINTS;
    if(equalsIgnoreCase("OK", s))
        return HTTP_OK;
    if(equalsIgnoreCase("Created", s))
        return HTTP_CREATED;
    if(equalsIgnoreCase("Accepted", s))
        return HTTP_ACCEPTED;
    if(equalsIgnoreCase("Non-Authoritative Information", s))
        return HTTP_NON_AUTHORITATIVE_INFORMATION;
    if(equalsIgnoreCase("No Content", s))
        return HTTP_NO_CONTENT;
    if(equalsIgnoreCase("Reset Content", s))
        return HTTP_RESET_CONTENT;
    if(equalsIgnoreCase("Partial Content", s))
        return HTTP_PARTIAL_CONTENT;
    if(equalsIgnoreCase("Multi-Status", s))
        return HTTP_MULTI_STATUS;
    if(equalsIgnoreCase("Already Reported", s))
        return HTTP_ALREADY_REPORTED;
    if(equalsIgnoreCase("IM Used", s))
        return HTTP_IM_USED;
    if(equalsIgnoreCase("Multiple Choices", s))
        return HTTP_MULTIPLE_CHOICES;
    if(equalsIgnoreCase("Moved Permanently", s))
        return HTTP_MOVED_PERMANENTLY;
    if(equalsIgnoreCase("Found", s))
        return HTTP_FOUND;
    if(equalsIgnoreCase("See Other", s))
        return HTTP_SEE_OTHER;
    if(equalsIgnoreCase("Not Modified", s))
        return HTTP_NOT_MODIFIED;
    if(equalsIgnoreCase("Use Proxy", s))
        return HTTP_USE_PROXY;
    if(equalsIgnoreCase("Temporary Redirect", s))
        return HTTP_TEMPORARY_REDIRECT;
    if(equalsIgnoreCase("Permanent Redirect", s))
        return HTTP_PERMANENT_REDIRECT;
    if(equalsIgnoreCase("Bad Request", s))
        return HTTP_BAD_REQUEST;
    if(equalsIgnoreCase("Unauthorized", s))
        return HTTP_UNAUTHORIZED;
    if(equalsIgnoreCase("Payment Required", s))
        return HTTP_PAYMENT_REQUIRED;
    if(equalsIgnoreCase("Forbidden", s))
        return HTTP_FORBIDDEN;
    if(equalsIgnoreCase("Not Found", s))
        return HTTP_NOT_FOUND;
    if(equalsIgnoreCase("Method Not Allowed", s))
        return HTTP_METHOD_NOT_ALLOWED;
    if(equalsIgnoreCase("Not Acceptable", s))
        return HTTP_NOT_ACCEPTABLE;
    if(equalsIgnoreCase("Proxy Authentication Required", s))
        return HTTP_PROXY_AUTH_REQUIRED;
    if(equalsIgnoreCase("Request Timeout", s))
        return HTTP_REQUEST_TIMEOUT;
    if(equalsIgnoreCase("Conflict", s))
        return HTTP_CONFLICT;
    if(equalsIgnoreCase("Gone", s))
        return HTTP_GONE;
    if(equalsIgnoreCase("Length Required", s))
        return HTTP_LENGTH_REQUIRED;
    if(equalsIgnoreCase("Precondition Failed", s))
        return HTTP_PRECONDITION_FAILED;
    if(equalsIgnoreCase("Payload Too Large", s))
        return HTTP_PAYLOAD_TOO_LARGE;
    if(equalsIgnoreCase("URI Too Long", s))
        return HTTP_URI_TOO_LONG;
    if(equalsIgnoreCase("Unsupported Media Type", s))
        return HTTP_UNSUPPORTED_MEDIA_TYPE;
    if(equalsIgnoreCase("Range Not Satisfiable", s))
        return HTTP_RANGE_NOT_SATISFIABLE;
    if(equalsIgnoreCase("Expectation Failed", s))
        return HTTP_EXPECTATION_FAILED;
    if(equalsIgnoreCase("I'm a teapot", s))
        return HTTP_IM_A_TEAPOT;
    if(equalsIgnoreCase("Misdirected Request", s))
        return HTTP_MISDIRECTED_REQUEST;
    if(equalsIgnoreCase("Unprocessable Content", s))
        return HTTP_UNPROCESSABLE_CONTENT;
    if(equalsIgnoreCase("Locked", s))
        return HTTP_LOCKED;
    if(equalsIgnoreCase("Failed Dependency", s))
        return HTTP_FAILED_DEPENDENCY;
    if(equalsIgnoreCase("Too Early", s))
        return HTTP_TOO_EARLY;
    if(equalsIgnoreCase("Upgrade Required", s))
        return HTTP_UPGRADE_REQUIRED;
    if(equalsIgnoreCase("Precondition Required", s))
        return HTTP_PRECONDITION_REQUIRED;
    if(equalsIgnoreCase("Too Many Requests", s))
        return HTTP_TOO_MANY_REQUESTS;
    if(equalsIgnoreCase("Request Header Fields Too Large", s))
        return HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE;
    if(equalsIgnoreCase("Unavailable For Legal Reasons", s))
        return HTTP_UNAVAILABLE_FOR_LEGAL_REASONS;
    if(equalsIgnoreCase("Internal Server Error", s))
        return HTTP_INTERNAL_SERVER_ERROR;
    if(equalsIgnoreCase("Not Implemented", s))
        return HTTP_NOT_IMPLEMENTED;
    if(equalsIgnoreCase("Bad Gateway", s))
        return HTTP_BAD_GATEWAY;
    if(equalsIgnoreCase("Service Unavailable", s))
        return HTTP_SERVICE_UNAVAILABLE;
    if(equalsIgnoreCase("Gateway Timeout", s))
        return HTTP_GATEWAY_TIMEOUT;
    if(equalsIgnoreCase("HTTP Version Not Supported", s))
        return HTTP_HTTP_VERSION_NOT_SUPPORTED;
    if(equalsIgnoreCase("Variant Also Negotiates", s))
        return HTTP_VARIANT_ALSO_NEGOTIATES;
    if(equalsIgnoreCase("Insufficient Storage", s))
        return HTTP_INSUFFICIENT_STORAGE;
    if(equalsIgnoreCase("Loop Detected", s))
        return HTTP_LOOP_DETECTED;
    if(equalsIgnoreCase("Not Extended", s))
        return HTTP_NOT_EXTENDED;
    if(equalsIgnoreCase("Network Authentication Required", s))
        return HTTP_NETWORK_AUTH_REQUIRED;

    return HTTP_INVALID_STATUS;
}
}