/* this is a naive byte-wise implementation of base64. */
/* use https://github.com/aklomp/base64  someday */

const unsigned char
base64_table_enc[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

/* In the lookup table below, note that the value for '=' (character 61) is
 * 254, not 255. This character is used for in-band signaling of the end of
 * the datastream, and we will use that later. The characters A-Z, a-z, 0-9
 * and + / are mapped to their "decoded" values. The other bytes all map to
 * the value 255, which flags them as "invalid input". */

const unsigned char
base64_table_dec[] =
{
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,     /*   0..15 */
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,     /*  16..31 */
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,     /*  32..47 */
     52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255, 255, 254, 255, 255,     /*  48..63 */
    255,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,     /*  64..79 */
     15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,     /*  80..95 */
    255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,     /*  96..111 */
     41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51, 255, 255, 255, 255, 255,     /* 112..127 */
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,     /* 128..143 */
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
};

static unsigned int
base64_size(unsigned int bin_size)
{
    unsigned int modulusLen = bin_size % 3;
    unsigned int pad = ((modulusLen&1) << 1) + ((modulusLen&2) >> 1); // 2 gives 1 and 1 gives 2, but 0 gives 0.
    return 4 * (bin_size + pad) / 3;
}

static unsigned int
base64(const unsigned char* bin, unsigned int bin_size, unsigned char* res, unsigned int res_size)
{
    if (!res || !bin)
    {
        return 0;
    }

    unsigned int modulusLen = bin_size % 3;
    unsigned int pad = ((modulusLen&1) << 1) + ((modulusLen&2) >> 1); // 2 gives 1 and 1 gives 2, but 0 gives 0.
    unsigned int outlen = 4 * (bin_size + pad) / 3;
    if (res_size < outlen)
    {
        return 0;
    }

    int byteNo; // I need this after the loop
    for (byteNo = 0; bin_size >= 3 && byteNo <= bin_size-3; byteNo += 3)
    {
        unsigned char b0 = bin[byteNo];
        unsigned char b1 = bin[byteNo+1];
        unsigned char b2 = bin[byteNo+2];
        *res++ = base64_table_enc[ b0 >> 2 ];
        *res++ = base64_table_enc[ ((0x03 & b0) << 4) + (b1 >> 4) ];
        *res++ = base64_table_enc[ ((0x0f & b1) << 2) + (b2 >> 6) ];
        *res++ = base64_table_enc[   0x3f & b2 ];
    }

    if (pad == 2)
    {
        *res++ = base64_table_enc[ bin[byteNo] >> 2 ];
        *res++ = base64_table_enc[ (0x3 & bin[byteNo]) << 4 ];
        *res++ = '=';
        *res++ = '=';
    }
    else if (pad == 1)
    {
        *res++ = base64_table_enc[         bin[byteNo] >> 2 ];
        *res++ = base64_table_enc[ ((0x3 & bin[byteNo]) << 4) + (bin[byteNo+1] >> 4) ];
        *res++ = base64_table_enc[ (0x0f & bin[byteNo+1]) << 2 ];
        *res++ = '=';
    }

    return outlen;
}

static unsigned int
unbase64_size(const unsigned char* ascii, unsigned int len)
{
    if (!ascii)
    {
        return 0;
    }

    int pad = 0;

    if (len < 2) // 2 accesses below would be OOB.
    {
        return 0;
    }
    if (ascii[len - 1] == '=') ++pad;
    if (ascii[len - 2] == '=') ++pad;

    if (len - pad < 2) // truncated, less than a byte
    {
        return 0;
    }

    return (3 * len / 4 - pad);
}

static unsigned int
unbase64(const unsigned char* ascii, unsigned int len, unsigned char *bin, unsigned int bin_size)
{
    if (!ascii || !bin)
    {
        return 0;
    }

    int pad = 0;

    if (len < 2) // 2 accesses below would be OOB.
    {
        return 0;
    }
    if (ascii[len - 1] == '=') ++pad;
    if (ascii[len - 2] == '=') ++pad;

    if (len - pad < 2) // truncated, less than a byte
    {
        return 0;
    }

    unsigned int outlen = 3 * len / 4 - pad;
    if (bin_size < outlen)
    {
        return 0;
    }

    int charNo;
    for (charNo = 0; len >= 4 + pad && charNo <= len - 4 - pad; charNo += 4)
    {
        int A = base64_table_dec[ascii[charNo]];
        int B = base64_table_dec[ascii[charNo+1]];
        int C = base64_table_dec[ascii[charNo+2]];
        int D = base64_table_dec[ascii[charNo+3]];

        *bin++ = (A<<2) | (B>>4);
        *bin++ = (B<<4) | (C>>2);
        *bin++ = (C<<6) | (D);
    }

    if (pad == 1 || charNo <= len - 3)  // single padding or truncated
    {
        int A = base64_table_dec[ascii[charNo]];
        int B = base64_table_dec[ascii[charNo+1]];
        int C = base64_table_dec[ascii[charNo+2]];

        *bin++ = (A<<2) | (B>>4);
        *bin++ = (B<<4) | (C>>2);
    }
    else if (pad == 2 || charNo <= len - 2)  // double padding or truncated
    {
        int A = base64_table_dec[ascii[charNo]];
        int B = base64_table_dec[ascii[charNo+1]];

        *bin++ = (A<<2) | (B>>4);
    }

    return outlen;
}
