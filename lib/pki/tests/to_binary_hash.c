
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// SHA1(iBSS.img3)= b51f78b42bb2d13281b4606b1446eb417625a657

unsigned char hex_to_bin(unsigned char hexchar)
{
    unsigned char val = hexchar - '0';
    if (val < 10)
        return val;
    val -= ('a' - '0' - 10);
    if (val < 16)
        return val;
    return 255;
}

int main(int argc, char *argv[])
{
    const unsigned char sha1oid[] = { 0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 
                                      0x03, 0x02, 0x1a, 0x05, 0x00, 0x04, 0x14 };
    const char prefix[] = "SHA1(", postfix[] = ")= ";
    char data[80];
    char *hash = fgets(data, sizeof(data), stdin);
    if (0 == strncmp(prefix, hash, sizeof(prefix) - 1)) {
        hash += sizeof(prefix) - 1; /* leave off null */
        hash = strstr(hash, postfix);
        if (!hash)
            return -1;
        hash += sizeof(postfix) - 1; /* leave off null */

        int i;
        for(i=0; i < sizeof(sha1oid); i++)
            putc(sha1oid[i], stdout);

        while (*hash && (*hash != '\n')) {
            unsigned char value = hex_to_bin(*hash++) << 4 | hex_to_bin(*hash++);
            putc(value, stdout);
        }
    }
}
