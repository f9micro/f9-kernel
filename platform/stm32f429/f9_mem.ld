/*
 * memory definitions
 */

MEMORY
{
	MFlash (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
	RamLoc2(rwx) : ORIGIN = 0x20020000, LENGTH =   64K
	RamLoc (rwx) : ORIGIN = 0x20000000, LENGTH =  112K
	RamCCM (rwx) : ORIGIN = 0x10000000, LENGTH =   64K
}
