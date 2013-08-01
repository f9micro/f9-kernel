#include <kip.h>
#include <platform/link.h>

kip_t kip __KIP = {
	.kernel_id = 0x00000000,

	.api_version.raw = 0x84 << 24 | 7 << 16,	/* L4 X.2, rev 7 */
	.api_flags.raw	= 0x00000000,		/* Little endian 32-bit */
};

/* Extra information on KIP */
char __kip_extra[CONFIG_KIP_EXTRA_SIZE] __KIP = "";
char *kip_extra = &__kip_extra[0];

kip_mem_desc_t *mem_desc = NULL;
