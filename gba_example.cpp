extern "C" {
#include </opt/devkitpro/libtonc/include/tonc_tte.h>
#include </opt/devkitpro/libtonc/include/tonc_video.h>
#include </opt/devkitpro/libtonc/include/tonc_irq.h>
}
#include "flash_filesystem.hpp"
#include "platform/platform.hpp"



void tte_init()
{
    irq_init(NULL);
    irq_add(II_VBLANK, NULL);
    REG_DISPCNT= DCNT_MODE0 | DCNT_BG0;

    tte_init_se(
        0,
        BG_CBB(0)|BG_SBB(31),
        0,
        CLR_YELLOW,
        14,
        NULL,
        NULL);

    pal_bg_bank[1][15]= CLR_RED;
    pal_bg_bank[2][15]= CLR_GREEN;
    pal_bg_bank[3][15]= CLR_BLUE;
    pal_bg_bank[4][15]= CLR_WHITE;
    pal_bg_bank[5][15]= CLR_MAG;
    pal_bg_bank[4][14]= CLR_GRAY;
}



int main(int argc, char** argv)
{
    flash_filesystem::Platform pfrm;

    tte_init();

    flash_filesystem::set_log_receiver([](const char* msg) {
                                           tte_write("fs: ");
                                           tte_write(msg);
                                           tte_write("\n");
                                      });


    auto stat = flash_filesystem::initialize(pfrm, 8);
    if (stat == flash_filesystem::InitStatus::failed) {
        tte_write("failed to init fs...\n");
        while (true) ; // Error, failed to init filesystem
    } else {
        tte_write("fs loaded!\n");
    }


    // Unlink files written by previous runs. You may still see the files in the
    // .sav output file, because this is a log-structured filesystem. To support
    // flash, we write to free space at the end of the filesystem, and don't
    // erase stuff until we actually need to.
    flash_filesystem::unlink_file(pfrm, "/chars/alphabet.txt");
    flash_filesystem::unlink_file(pfrm, "/chars/uppercase.txt");


    flash_filesystem::Vector<char> buffer;
    for (int i = 0; i < 26; ++i) {
        buffer.push_back('a' + i);
    }

    flash_filesystem::store_file_data_text(pfrm, "/chars/alphabet.txt", buffer);

    buffer.clear();



    flash_filesystem::read_file_data_text(pfrm, "/chars/alphabet.txt", buffer);
    for (char& c : buffer) {
        c -= 'a' - 'A';
    }
    flash_filesystem::store_file_data_text(pfrm, "/chars/uppercase.txt", buffer);


    if (flash_filesystem::file_size(pfrm, "/chars/uppercase.txt") == 26) {
        tte_write("filesize check passed!\n");
    }


    tte_write("walk directory /chars/\n");
    flash_filesystem::walk_directory(pfrm, "/chars/",
                                     [](const char* path) {
                                         tte_write(path);
                                         tte_write("\n");
                                     });



    while (1) ; // this is a gba, don't return from main!

    return 0;
}
