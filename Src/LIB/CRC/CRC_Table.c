/*
 * CRC_Table.c
 *
 *  Created on: Aug 16, 2023
 *      Author: Ali Emad
 */

#include "stdint.h"

/*
 * The following table was generated using the code commented below:
 *
 *
 *		#include <stdio.h>
 *
 *		typedef unsigned char u8;
 *
 *		#define MSB(x) (x >> 7)
 *
 *		u8 p = 0x89;
 *
 *		u8 get_crc(u8 x)
 *		{
 *			u8 c = x;
 *			if (MSB(c) == 1)
 *				c = c ^ p;
 *
 *			for (u8 i = 0; i < 7; i++)
 *			{
 *				c = c << 1;
 *				if (MSB(c) == 1)
 *					c = c ^ p;
 *			}
 *
 *			return c;
 *		}
 *
 *		int main() {
 *			for (int i = 0; i < 256; i++)
 *			{
 *				printf("%d, ", get_crc(i));
 *				if ((i+1) % 4 == 0)
 *					printf("\n");
 *			}
 *
 *			return 0;
 *		}
 */
const uint8_t pucCrc7Table[256] = {
	0, 9, 18, 27,
	36, 45, 54, 63,
	72, 65, 90, 83,
	108, 101, 126, 119,
	25, 16, 11, 2,
	61, 52, 47, 38,
	81, 88, 67, 74,
	117, 124, 103, 110,
	50, 59, 32, 41,
	22, 31, 4, 13,
	122, 115, 104, 97,
	94, 87, 76, 69,
	43, 34, 57, 48,
	15, 6, 29, 20,
	99, 106, 113, 120,
	71, 78, 85, 92,
	100, 109, 118, 127,
	64, 73, 82, 91,
	44, 37, 62, 55,
	8, 1, 26, 19,
	125, 116, 111, 102,
	89, 80, 75, 66,
	53, 60, 39, 46,
	17, 24, 3, 10,
	86, 95, 68, 77,
	114, 123, 96, 105,
	30, 23, 12, 5,
	58, 51, 40, 33,
	79, 70, 93, 84,
	107, 98, 121, 112,
	7, 14, 21, 28,
	35, 42, 49, 56,
	65, 72, 83, 90,
	101, 108, 119, 126,
	9, 0, 27, 18,
	45, 36, 63, 54,
	88, 81, 74, 67,
	124, 117, 110, 103,
	16, 25, 2, 11,
	52, 61, 38, 47,
	115, 122, 97, 104,
	87, 94, 69, 76,
	59, 50, 41, 32,
	31, 22, 13, 4,
	106, 99, 120, 113,
	78, 71, 92, 85,
	34, 43, 48, 57,
	6, 15, 20, 29,
	37, 44, 55, 62,
	1, 8, 19, 26,
	109, 100, 127, 118,
	73, 64, 91, 82,
	60, 53, 46, 39,
	24, 17, 10, 3,
	116, 125, 102, 111,
	80, 89, 66, 75,
	23, 30, 5, 12,
	51, 58, 33, 40,
	95, 86, 77, 68,
	123, 114, 105, 96,
	14, 7, 28, 21,
	42, 35, 56, 49,
	70, 79, 84, 93,
	98, 107, 112, 121
};

/*
 * The following table was generated using the code commented below:
 *
 *
 *      include <stdio.h>
 *
 *      typedef unsigned char u8;
 *      typedef unsigned short u16;
 *      typedef unsigned int u32;
 *
 *      #define MSB(x) (x >> 16)
 *
 *      u32 p = 0x11021;
 *
 *      u16 get_crc(u16 x)
 *      {
 *      	u32 c = x;
 *      	if (MSB(c) == 1)
 *      		c = c ^ p;
 *
 *      	for (u8 i = 0; i < 16; i++)
 *      	{
 *      		c = c << 1;
 *      		if (MSB(c) == 1)
 *      			c = c ^ p;
 *      	}
 *
 *      	return c;
 *      }
 *
 *      int main() {
 *      	for (int i = 0; i < 256; i++)
 *      	{
 *      		printf("%d, ", get_crc(i));
 *      		if ((i+1) % 4 == 0)
 *      			printf("\n");
 *      	}
 *
 *      	return 0;
 *      }
 */
const uint16_t pusCrc16Table[256] = {
	0, 4129, 8258, 12387,
	16516, 20645, 24774, 28903,
	33032, 37161, 41290, 45419,
	49548, 53677, 57806, 61935,
	4657, 528, 12915, 8786,
	21173, 17044, 29431, 25302,
	37689, 33560, 45947, 41818,
	54205, 50076, 62463, 58334,
	9314, 13379, 1056, 5121,
	25830, 29895, 17572, 21637,
	42346, 46411, 34088, 38153,
	58862, 62927, 50604, 54669,
	13907, 9842, 5649, 1584,
	30423, 26358, 22165, 18100,
	46939, 42874, 38681, 34616,
	63455, 59390, 55197, 51132,
	18628, 22757, 26758, 30887,
	2112, 6241, 10242, 14371,
	51660, 55789, 59790, 63919,
	35144, 39273, 43274, 47403,
	23285, 19156, 31415, 27286,
	6769, 2640, 14899, 10770,
	56317, 52188, 64447, 60318,
	39801, 35672, 47931, 43802,
	27814, 31879, 19684, 23749,
	11298, 15363, 3168, 7233,
	60846, 64911, 52716, 56781,
	44330, 48395, 36200, 40265,
	32407, 28342, 24277, 20212,
	15891, 11826, 7761, 3696,
	65439, 61374, 57309, 53244,
	48923, 44858, 40793, 36728,
	37256, 33193, 45514, 41451,
	53516, 49453, 61774, 57711,
	4224, 161, 12482, 8419,
	20484, 16421, 28742, 24679,
	33721, 37784, 41979, 46042,
	49981, 54044, 58239, 62302,
	689, 4752, 8947, 13010,
	16949, 21012, 25207, 29270,
	46570, 42443, 38312, 34185,
	62830, 58703, 54572, 50445,
	13538, 9411, 5280, 1153,
	29798, 25671, 21540, 17413,
	42971, 47098, 34713, 38840,
	59231, 63358, 50973, 55100,
	9939, 14066, 1681, 5808,
	26199, 30326, 17941, 22068,
	55628, 51565, 63758, 59695,
	39368, 35305, 47498, 43435,
	22596, 18533, 30726, 26663,
	6336, 2273, 14466, 10403,
	52093, 56156, 60223, 64286,
	35833, 39896, 43963, 48026,
	19061, 23124, 27191, 31254,
	2801, 6864, 10931, 14994,
	64814, 60687, 56684, 52557,
	48554, 44427, 40424, 36297,
	31782, 27655, 23652, 19525,
	15522, 11395, 7392, 3265,
	61215, 65342, 53085, 57212,
	44955, 49082, 36825, 40952,
	28183, 32310, 20053, 24180,
	11923, 16050, 3793, 7920
};
