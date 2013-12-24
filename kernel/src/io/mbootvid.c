#include <types.h>
#include <err/error.h>
#include <video.h>

static struct multiboot_fb_tag *fb_tag = 0;

static void store_vid_tag(struct multiboot_header_tag *mboot_tag)
{
	fb_tag = (struct multiboot_fb_tag *)find_multiboot_table(mboot_tag, 8);
	if(!fb_tag)
		kerror(ERR_MEDERR, "Mutiboot framebuffer tag not found, trying to use default video mode");
}

void init_video(struct multiboot_header_tag* mboot_tag)
{
	store_vid_tag(mboot_tag);
	
	kerror(ERR_INFO, "Video type reported by GRUB: %d", (u32)fb_tag->fb_type);
	kerror(ERR_INFO, "  -> Resolution: %dx%d", (u32)fb_tag->fb_width, (u32)fb_tag->fb_height);
	kerror(ERR_INFO, "  -> Address:  0x%08X", (u32)fb_tag->fb_addr);
	kerror(ERR_INFO, "  -> Pitch:    0x%X", (u32)fb_tag->fb_pitch);
	kerror(ERR_INFO, "  -> BPP:        %d", (u32)fb_tag->fb_bpp);
	
	if(fb_tag->fb_addr)
	{
		u8 *ad = (u8 *)(ptr_t)fb_tag->fb_addr;
		ad[0] = 1;
		ad[1] = 2;
	}
}

void dumb_unused_test(int ha, ...)
{
	__builtin_va_list vl;
	__builtin_va_start(vl, ha);
	(void)ha;
	__builtin_va_end(vl);
}