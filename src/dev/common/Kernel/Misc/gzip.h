#pragma once

namespace rw
{
	extern int gzipDecompress(const void* source, unsigned sourceLen, void* dest, unsigned* destLen);
	extern int gzipCompress(const void* source, unsigned sourceLen, void* dest, unsigned* destLen, int level);
}