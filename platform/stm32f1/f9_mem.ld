/*
 * memory definitions
 */

MEMORY
{
	MFlash (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
	RamLoc (rwx) : ORIGIN = 0x20000000, LENGTH =  112K
}
