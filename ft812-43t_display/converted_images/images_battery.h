#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"

#define RAM_IMAGES_BATTERY 531764

/*Placed in external flash*/
CY_SECTION(".cy_xip_code") __attribute__((used))
uint8_t images_battery[] = 
{
	/*('file properties: ', 'resolution ', 52, 'x', 42, 'format ', 'ARGB1555', 'stride ', 104, ' total size ', 4368)*/ 120,156,237,215,193,10,192,32,12,3,80,255,255,155,61,12,60,9,154,181,117,77,113,144,120,78,159,40,200,214,218,146,190,44,43,241,198,174,99,245,226,13,212,57,113,60,157,221,20,75,241,55,228,196,29,116,151,25,171,194,152,37,190,211,157,183,246,37,232,20,185,14,51,114,228,228,59,120,150,156,60,231,237,37,180,230,163,134,156,255,57,231,193,179,228,220,224,120,118,32,71,78,173,195,255,238,173,250,95,168,115,216,214,200,3,59,243,241,43,
};