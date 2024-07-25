#include "sdhc.h"

// Adapted from:
// https://os.mbed.com/users/hudakz/code/SDCard/file/d08c053d5a35/main.cpp/

// Physical block device, can be any device that supports the BlockDevice API
SDBlockDevice   blockDevice(SD_CARD_SPI_MOSI, SD_CARD_SPI_MISO, SD_CARD_SPI_SCK, SD_CARD_SPI_CS);
// File system declaration
FATFileSystem   fileSystem("fs");

uint8_t FILESYSTEM_MOUNTED       = 0;

int sdcard_mount();

int filesystem_mount(void)
{
    // Try to mount the filesystem 
    uint8_t err = sdcard_mount();
    if(!err)
        FILESYSTEM_MOUNTED           = 1;

    return err;

}

uint8_t is_mounted(void)
{
    return FILESYSTEM_MOUNTED;
}

int sd_cleanup(void)
{
    // Tidy up
    uint8_t err = 0;
    err = sd_unmount();
    _debug("Deinitializing the block device... ");
    err = deinit_bd();

    if(! err )
        FILESYSTEM_MOUNTED   = 0;

    return err;
}

int sdcard_mount()
{
    _debug("Mounting the filesystem... ");
    fflush(stdout);

    int err = fileSystem.mount(&blockDevice);
    _ack("%s\n\r", (err ? "Fail :(" : "OK"));
    if (err) {
        // Reformat if we can't mount the filesystem
        // this should only happen on the first boot
        _debug("No filesystem found, formatting... ");
        fflush(stdout);
        err = fileSystem.reformat(&blockDevice);
        _ack("%s\n\r", (err ? "Fail :(" : "OK"));
        if (err) {
            _ack("error: %s (%d)\n\r", strerror(-err), err);
            return -2;
        }
    }
    return err;
}

int open_file(const char* filepath, mFile *myFile)
{
    int err = 0;
    _debug("Opening \"/fs/numbers.txt\"... ");
    fflush(stdout);
 
    myFile->f = fopen("/fs/numbers.txt", "r+");
    _ack("%s\n\r", (!(myFile->f) ? "Fail :(" : "OK"));
    if (!(myFile->f)) {
        // Create the numbers file if it doesn't exist
        _warning("No file found, creating a new file... ");
        fflush(stdout);
        myFile->f = fopen("/fs/numbers.txt", "w+");
        _ack("%s\n\r", (!(myFile->f) ? "Fail :(" : "OK"));
        if (!(myFile->f)) {
            error("error: %s (%d)\n\r", strerror(errno), -errno);
        }
 
        for (int i = 0; i < 10; i++) {
            _ack("\rWriting numbers (%d/%d)... ", i, 10);
            fflush(stdout);
            err = fprintf((myFile->f), "    %d\n", i);
            if (err < 0) {
                _ack("Fail :(\n\r");
                _error("error: %s (%d)\n\r", strerror(errno), -errno);
            }
        }
 
        _debug("\rWriting numbers (%d/%d)... OK\n\r", 10, 10);
 
        _debug("Seeking file... ");
        fflush(stdout);
        err = fseek((myFile->f), 0, SEEK_SET);
        _ack("%s\n\r", (err < 0 ? "Fail :(" : "OK"));
        if (err < 0) {
            error("error: %s (%d)\n\r", strerror(errno), -errno);
        }
    }

    return err;
}

int close_file(mFile *myFile)
{
    int err = 0;
    _debug("Closing \"/fs/numbers.txt\"... ");
    fflush(stdout);
    err = fclose((myFile->f));
    _ack("%s\n\r", (err < 0 ? "Fail :(" : "OK"));
    if (err < 0) {
        _error("error: %s (%d)\n\r", strerror(errno), -errno);
    }
    return err;
}

int display_root_directory()
{
    int err = 0;
    _debug("Opening the root directory... ");
    fflush(stdout);
 
    DIR*    d = opendir("/fs/");
    _ack("%s\n\r", (!d ? "Fail :(" : "OK"));
    if (!d) {
        _error("error: %s (%d)\n\r", strerror(errno), -errno);
    }
 
    _debug("root directory:\n\r");
    while (true) {
        struct dirent*  e = readdir(d);
        if (!e) {
            break;
        }
 
        _debug("    %s\n\r", e->d_name);
    }
 
    _debug("Closing the root directory... ");
    fflush(stdout);
    err = closedir(d);
    _ack("%s\n\r", (err < 0 ? "Fail :(" : "OK"));
    if (err < 0) {
        error("error: %s (%d)\n\r", strerror(errno), -errno);
    }

    return err;
}

int display_file(mFile *myFile)
{
    int err = 0;
    _debug("Opening \"/fs/numbers.txt\"... ");
    fflush(stdout);
    (myFile->f) = fopen("/fs/numbers.txt", "r");
    _ack("%s\n\r", (!(myFile->f) ? "Fail :(" : "OK"));
    if (!(myFile->f)) {
        error("error: %s (%d)\n\r", strerror(errno), -errno);
    }
 
    _debug("numbers:\n\r");
    while (!feof((myFile->f))) {
        int c = fgetc((myFile->f));
        _debug("%c", c);
    }
 
    _debug("\rClosing \"/fs/numbers.txt\"... ");
    fflush(stdout);
    err = fclose((myFile->f));
    _ack("%s\n\r", (err < 0 ? "Fail :(" : "OK"));
    if (err < 0) {
        error("error: %s (%d)\n\r", strerror(errno), -errno);
    }
    return err;
}

// Only for testing purposes
int sd_increment(mFile *myFile)
{
    int err = 0;
    for (int i = 0; i < 10; i++) {
        _debug("\rIncrementing numbers (%d/%d)... ", i, 10);
        fflush(stdout);
 
        // Get current stream position
        long    pos = ftell((myFile->f));
 
        // Parse out the number and increment
        int32_t number;
        fscanf((myFile->f), "%ld", &number);
        number += 1;
 
        // Seek to beginning of number
        fseek((myFile->f), pos, SEEK_SET);
 
        // Store number
        fprintf((myFile->f), "    %ld\n", number);
 
        // Flush between write and read on same file
        fflush((myFile->f));
    }
    return err;
}

int sd_unmount()
{
    int err = 0;
    _debug("Unmounting... ");
    fflush(stdout);
    err = fileSystem.unmount();
    _ack("%s\n\r", (err < 0 ? "Fail :(" : "OK"));
    if (err < 0) {
        error("error: %s (%d)\n\r", strerror(-err), err);
    }
    return err;
}

int deinit_bd()
{
    fflush(stdout);
 
    int err = blockDevice.init();
    _ack("%s\n\r", (err ? "Fail :(" : "OK"));
    if (err) {
        error("error: %s (%d)\n\r", strerror(-err), err);
    }
 
    _debug("Erasing the block device... ");
    fflush(stdout);
    err = blockDevice.erase(0, blockDevice.size());
    _ack("%s\n\r", (err ? "Fail :(" : "OK"));
    if (err) {
        error("error: %s (%d)\n\r", strerror(-err), err);
    }
 
    _debug("Deinitializing the block device... ");
    fflush(stdout);
    err = blockDevice.deinit();
    _ack("%s\n\r", (err ? "Fail :(" : "OK"));
    if (err) {
        error("error: %s (%d)\n\r", strerror(-err), err);
    }
    return err;
}

int sd_card_unit_test(void)
{
    mFile myBestFile;
    myBestFile.filepath = "/fs/numbers.txt";
 
    // Open the numbers file
    open_file("/fs/numbers.txt", &myBestFile);
 
    // Go through and increment the numbers
    sd_increment(&myBestFile);
 
    // Close the file which also flushes any cached writes
    close_file(&myBestFile);
 
    // Display the root directory
    display_root_directory();
 
    // Display the numbers file
    display_file(&myBestFile);

    return 0;
}