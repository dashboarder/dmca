#include <lib/cksum.h>
#include <sys/types.h>
#include <stdint.h>

/* Table of CRCs of all 8-bit messages. */
static uint32_t crc32_table[256];
   
/* Flag: has the table been computed? Initially false. */
static int crc32_table_computed = 0;
   
/* Make the table for a fast CRC. */
static void make_crc32_table(void)
{
	uint32_t c;
	int n, k;
   
	for (n = 0; n < 256; n++) {
		c = (uint32_t) n;
		for (k = 0; k < 8; k++) {
			if (c & 1)
				c = 0xedb88320L ^ (c >> 1);
			else
				c = c >> 1;
		}
		crc32_table[n] = c;
	}
	crc32_table_computed = 1;
}
  
/* Update a running CRC with the bytes buf[0..len-1]--the CRC
   should be initialized to all 1's, and the transmitted value
   is the 1's complement of the final running CRC (see the
   crc32() routine below). */
   
uint32_t update_crc32(uint32_t crc, const uint8_t *buf,
			 int len)
{
	uint32_t c = crc;
	int n;
   
	if (!crc32_table_computed)
		make_crc32_table();
	for (n = 0; n < len; n++) {
		c = crc32_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
	}
	return c;
}
   
/* Return the CRC of the bytes buf[0..len-1]. */
uint32_t crc32(const uint8_t *buf, int len)
{
	return update_crc32(0xffffffffL, buf, len) ^ 0xffffffffL;
}
