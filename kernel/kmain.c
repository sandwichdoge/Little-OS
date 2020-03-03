#include "serial.h"
#include "framebuffer.h"
#include "interrupt.h"
#include "pic.h"
#include "shell.h"
#include "syscall.h"
#include "paging.h"
#include "multiboot.h"
#include "kinfo.h"
#include "kheap.h"
#include "malloc.h"
#include "utils/debug.h"
#include "utils/string.h"
#include "stdtype.h"

char *msg_fb_test = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Nisi scelerisque eu ultrices vitae. Cursus risus at ultrices mi tempus. Lacus vel facilisis volutpat est velit. Dui nunc mattis enim ut tellus elementum sagittis vitae et. Adipiscing elit duis tristique sollicitudin nibh sit amet. Ornare arcu odio ut sem nulla pharetra. Ultrices gravida dictum fusce ut placerat orci nulla pellentesque. Fames ac turpis egestas sed tempus urna. Sit amet cursus sit amet dictum.Dignissim convallis aenean et tortor at risus viverra. Dui sapien eget mi proin sed libero. Nunc vel risus commodo viverra maecenas accumsan lacus vel facilisis. Ut etiam sit amet nisl purus in. Eu lobortis elementum nibh tellus molestie. Arcu ac tortor dignissim convallis. Ultrices tincidunt arcu non sodales neque sodales ut. Ultrices vitae auctor eu augue. Ut tellus elementum sagittis vitae et. Bibendum arcu vitae elementum curabitur. Sed euismod nisi porta lorem mollis aliquam. Tellus rutrum tellus pellentesque eu tincidunt. Cursus metus aliquam eleifend mi. Faucibus et molestie ac feugiat sed lectus vestibulum mattis ullamcorper. Sed id semper risus in hendrerit gravida rutrum quisque. In ornare quam viverra orci sagittis eu volutpat. Egestas sed tempus urna et pharetra pharetra massa massa ultricies.Vulputate eu scelerisque felis imperdiet proin fermentum. Mattis nunc sed blandit libero volutpat sed cras ornare. Nibh mauris cursus mattis molestie a iaculis at. Diam volutpat commodo sed egestas egestas fringilla phasellus. Semper risus in hendrerit gravida rutrum. Pulvinar etiam non quam lacus suspendisse faucibus interdum posuere lorem. At elementum eu facilisis sed odio morbi quis commodo. Molestie ac feugiat sed lectus vestibulum. Luctus accumsan tortor posuere ac ut consequat semper viverra nam. Elementum nisi quis eleifend quam adipiscing vitae proin. At volutpat diam ut venenatis tellus in metus. Facilisis magna etiam tempor orci eu. Ornare arcu dui vivamus arcu felis bibendum. Libero id faucibus nisl tincidunt eget nullam non nisi est.Proin sed libero enim sed faucibus turpis in. Ut eu sem integer vitae justo. Duis ut diam quam nulla. Iaculis nunc sed augue lacus. Diam maecenas ultricies mi eget. Molestie nunc non blandit massa enim nec dui. Ac tortor vitae purus faucibus. Nisi scelerisque eu ultrices vitae auctor eu augue ut. Mollis nunc sed id semper risus in hendrerit gravida rutrum. Ipsum dolor sit amet consectetur adipiscing elit. Eu tincidunt tortor aliquam nulla facilisi cras. Vel eros donec ac odio tempor orci dapibus ultrices in. Dignissim convallis aenean et tortor. Et malesuada fames ac turpis egestas sed. Elementum facilisis leo vel fringilla. Etiam tempor orci eu lobortis elementum nibh tellus molestie. Aliquam faucibus purus in massa tempor. Hendrerit gravida rutrum quisque non tellus orci ac auctor augue. Purus non enim praesent elementum facilisis leo vel fringilla.";

void test_memory_32bit_mode() {
    volatile unsigned char *p = (volatile unsigned char *)(0xc0000000 + 3000000); // 32MB for testing 32-bit mode
    *p = 55;

    if (*p == 55) {
        write_cstr("Memtest success.", 80);
    } else {
        write_cstr("Memtest failed.", 80);
        _dbg_break();
    }
}

void halt() {
    asm("hlt");
}

void call_user_module(multiboot_info_t *mbinfo) {
    struct multiboot_mod_list *mods = (struct multiboot_mod_list *)(mbinfo->mods_addr + 0xc0000000);
    unsigned int mcount = mbinfo->mods_count;

    if (mcount > 0) {
        unsigned int prog_addr = (mods->mod_start + 0xc0000000);

        typedef void (*call_module_t)(void);
        call_module_t start_program = (call_module_t)prog_addr;

        start_program();
    }
}

void kmain(unsigned int ebx) {
// First thing first, gather all info about our hardware capabilities, store it in kinfo singleton
#ifdef WITH_GRUB_MB
    multiboot_info_t *mbinfo = (multiboot_info_t *)ebx;
    kinfo_init((multiboot_info_t *)ebx);
#else
    kinfo_init(NULL);
#endif

    serial_defconfig(SERIAL_COM1_BASE);

// Setup heap
    kheap_init();

// Setup paging
    write_cstr("Setting up paging..", 80);
    paging_init();

// Setup interrupts
    write_cstr("Setting up interrupts..", 0);
    interrupt_init_idt();
    pic_init();

    malloc_init();

// Perform memory tests
    test_memory_32bit_mode();

// Call user program
#ifdef WITH_GRUB_MB
    call_user_module(mbinfo);
#endif

    _dbg_serial("Hello, world\n");

// Enter I/O shell
    shell_main();

    while (1) {
        
    }
}