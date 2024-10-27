#include "SP/sparse/mot.h"

long long sdbm_hash(const char *str) {
    long long hash = 0;
    int c;
    while ((c = *str++)) {
        hash = c + (hash << 6) + (hash << 16) - hash; // hash * 65599 + c
    }

    return hash;
}

long long sdbmHashGPT(const char* str)
{
	SPulong hash = sdbm_hash(str);
    SPbyte buffer[4];
    // Split the hash into 8 bytes
    for (int i = 0; i < 8; i++) {
        buffer[i] = (hash >> (i * 8)) & 0xFF; // Get the i-th byte
    }
	
	long long output = 0;
	memcpy(&output, buffer, sizeof(SPbyte) * 4);
	return output;
}

long long sdbmHashGithub(const char* str)
{
	SPsize len = strlen(str);
	register unsigned long n = 0;
	while (len--)
		n = *str++ + 65599 * n;
	return n;
}