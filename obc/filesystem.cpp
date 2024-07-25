#include "filesystem.h"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include "wd.h"
#include "submodule.h"

#define MAX_METAFILE_COUNT 16

#define UPLINK_FOLDER "/fs/uplink"
#define TESTFILE "/fs/test.test"
#define UPBLOCK_MAX_LENGTH 57
#define METAFILE_EXTENSION ".up"

// Flag to show that uplink is initialized and ready to receive
uint8_t UPLINK_ACTIVE = 0;
char metafilePath[128];
char Uplink_file_name[64];
extern DigitalIn sd_card_connected;
extern UnbufferedSerial   _pc;
void NoActiveFile();

int CountBlocks(int fd);
inline bool File_exists(const std::string& name);


int readFilesize(int fd)
{
    struct stat st;
    off_t file_size;
    
    if (fstat(fd, &st) != 0){
        printf("fstat error\r\n");
    }

    file_size = st.st_size;
    if (file_size == -1) {
    /* Handle error */
    }

    return (int) file_size;
}


int readFilesize(char *filepath)
{
    int fd = open(filepath, O_RDONLY);
    int length = readFilesize(fd);
    close(fd);
    return length;
}

int test_filesystem()
{
    FILE *f;
    // Test SD-card physical connection
    uint8_t status = 0;
    status = !sd_card_connected; // read the GPIO in

    if(status)
    {
        _error("No SD-card detected.\n\r");
        return FILESYSTEM_ERROR_NO_SDCARD;
    } 

    char str[32];
    memset(str, 0, 32);
    strcpy(str, "Lorem ipsum dolor sit amet");
    f = fopen(TESTFILE, "w");
    if(f == NULL)
    {
        _error("Failed to write to SD-card.\n\r");
        return FILESYSTEM_ERROR_WRITE_FAIL;
    }
    fwrite(str, sizeof(char), strlen(str), f);
    fclose(f);

    f = fopen(TESTFILE, "r");
    
    char str2[32];
    memset(str2, 0, 32);
    fread(str2, sizeof(char), strlen(str), f);
    fclose(f);
    if(remove(TESTFILE))
    {
        _error("Failed to delete the test file.\n\r");
        return FILESYSTEM_ERROR_WRITE_FAIL;
    }
    if(strcmp(str, str2))
    {
        _error("Failed to read from SD-card.\n\r");
       return FILESYSTEM_ERROR_READ_FAIL; 
    }

    return FILESYSTEM_NOMINAL;
}

inline bool File_exists(const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }   
}

int n_latest_files(unsigned int n, char most_recent_images[20][50])
{
    DIR *d = NULL;
    struct dirent *dir = NULL;
    d = opendir("/fs/mydir");
    unsigned int file_count = 0;

    int k = 0, i = 0;

    if (d)
    {

        // Count files :)
        while ((dir = readdir(d)) != NULL)
        {
            if (dir->d_type == DT_REG)
            { /* If th entry is a regular file */
                file_count++;
            }
        }

        closedir(d);

        n > file_count ? n = file_count : n;
        n > 20 ? n = 20 : n;
        
        // Re-open for name lists
        d = opendir("/fs/mydir");

        // Print n latest
        while ((dir = readdir(d)) != NULL)
        {
            if ( i >= (file_count - n))
            {
                char* curr_img = dir->d_name;
                sprintf(most_recent_images[k], "%s", curr_img);
                feed_the_dog();
                k++;
            }
            i++;
        }
        closedir(d);
    } else {
        n = FILE_OPEN_FAILED;
    }

    return n;

}

int latest_file(char* most_recent_image)
{
    DIR *d = NULL;
    struct dirent *dir = NULL;
    d = opendir("/fs/mydir");
    unsigned int file_count = 0;

    int k = 0, i = 0;

    if (d)
    {

        while ((dir = readdir(d)) != NULL)
        {
            if (dir->d_type == DT_REG)
            {
                sprintf(most_recent_image, "%s", dir->d_name);
            }
        }

        closedir(d);
    } else {
        return FILE_OPEN_FAILED;
    }

    return 0;

}

int get_size_in_chunks(char* image_name)
{
    char path[128];
    memset(path, 0, 128);
    sprintf(path, "/fs/mydir/%s", image_name);
    size_t bytesRead = 0;

    FILE *fp = fopen(path, "rb");
    int i = 0;

    if(fp == NULL)
    {
        _error("Can't open file for reading: %s\n\r", path);
        return FILE_OPEN_FAILED;
    }

    char buf[53];
    while ((bytesRead = fread(buf, 1, sizeof(buf), fp)) > 0)
    {     
        feed_the_dog();
        i++;
    }
    fclose(fp);

    return i;
}


void CreateFilePath(char *destination, char *filename, const char *extension)
{
    sprintf(destination, "%s/%s%s", UPLINK_FOLDER, filename, extension);
}

void CreateUplinkFiles(char *_filepath, char *filename, int length)
{
    FILE *f;
    char metaline[64];
    memset(metaline, 0, 64);
    sprintf(metaline, "%d %s", length, filename);
    f = fopen(_filepath, "wb");
    if(f == NULL) {
        error("Could not open file for write\n");
    }
    fwrite(metaline, sizeof(char), 64, f);
    fclose(f);
}

void ReadMetaLine(char *dest, char *filepath)
{
    FILE *f = fopen(filepath, "r");
    fread(dest, sizeof(char), 64, f);
    fclose(f);
}


uint16_t GetBlockIndex(char *block)
{
    uint16_t val = block[0] | (uint16_t) block[1] << 8;
    return val; 
}

uint8_t GetBlockLength(char *block)
{
    uint8_t val = block[2];
    return val;
}



int SaveBlock(char *data, uint16_t block_number, uint8_t data_len) {

    char newblock[64];
    memset(newblock, 0, 64);
    memcpy(newblock, &block_number, 2);
    memcpy(newblock+2, &data_len, 1);
    memcpy(newblock+3, data, data_len);
    
    int f = open(metafilePath, O_RDWR);
    int length = CountBlocks(f);
    
    if(block_number > length)
    {
        char emptyBlock[64];
        memset(emptyBlock, 0, 64);
        lseek(f, 0, SEEK_END);
        for(int i = length; i<block_number; i++)
        {
            write(f, emptyBlock, 64);
        }
    }

    lseek(f, 64*(block_number+1), SEEK_SET);

    write(f, newblock, 64);
    
    close(f);

    return 0;
}



void FindMissingBlocks(int *array, int *dest, int length, int max)
{
    if(length < 0) return;
    
    int highestValue = array[length-1];
    
    int addedValues = 0;
    
    int index = 0;
    
    for(int i=0; i<highestValue; i++)
    {
        if(array[index] != i)
        {
            dest[addedValues++] = i;
        }else if(array[index] == i)
        {
            index++;
        }
        if(addedValues >= max) return;
    }
    
    dest[addedValues] = highestValue + 1;
}

void print_int_array(int *arr, int length)
{
    for(int i = 0; i < length; i++)
    {
        printf("%d", arr[i]);
        if(i<length-1) printf(", ");
    }
    printf("\r\n");
}

void NEXT_BLOCKS()
{
    int now = ms_since_boot();
    #define MAX_INDICES 26

    int f = open(metafilePath, O_RDONLY);
    int length = CountBlocks(f);


    // gather missing indices to array. Initialize with -1 to show missing values
    uint16_t missingBlocks[MAX_INDICES];
    for(int i = 0; i<MAX_INDICES; i++)
    {
        missingBlocks[i] = (uint16_t) -1;
    }



    // add missing indices to the array by checking invalid lines
    int missingBlocksIndex = 0;
    lseek(f, 64, SEEK_SET);
    for(int i = 0; i<length; i++)
    {
        char block[64];
        read(f, block, 64);
        int blockIndex = GetBlockIndex(block);
        int blockLength = GetBlockLength(block);

        if(blockIndex != i || blockLength == 0)
        {
            missingBlocks[missingBlocksIndex] = i;
            missingBlocksIndex++;
        }

        if(missingBlocksIndex >= MAX_INDICES)
        {
            break;
        }
    }

    // if the file ended before max number of lines was found, add the next number
    if(missingBlocksIndex < MAX_INDICES)
    {
        missingBlocks[missingBlocksIndex] = length;
    }

    // remove unnecessary trailing values
    for(int i = missingBlocksIndex-1; i>0; i--)
    {
        if(missingBlocks[i] == missingBlocks[i-1] + 1)
        {
            missingBlocks[i] = -1;
        }else{
            break;
        }
    }

    char msg[2*MAX_INDICES];
    int ValidBlocks = 0;
    int missingBlockIndex = 0;
    for(int i = 0; i<MAX_INDICES*2; i+=2)
    {
        uint16_t val = missingBlocks[missingBlockIndex++];
        if(val == (uint16_t) -1) break;
        ValidBlocks++;
        msg[i] = val & 0xff;
        msg[i+1] = (val >> 8) & 0xff;
    }

    sendPacket(FILETRANSFER, 8, msg, (sizeof(uint16_t))*ValidBlocks);

    printf("%d ms\r\n", ms_since_boot()-now);
}


void UPLINK_STATUS()
{
    int16_t block_count = 0;
    if(!UPLINK_ACTIVE)
    {
        NoActiveFile();
        block_count = -1;
        return;
    }

    char msg[2];
    memcpy(msg, &block_count, 2);
    sendPacket(FILETRANSFER, 3, msg, sizeof(msg));

}

void NoActiveFile()
{
    char msg[32];
    sprintf(msg, "No active file set");
    sendPacket(FILETRANSFER, 9, msg, sizeof(msg));
}

int CountBlocks(char *filepath)
{
    int fd = open(filepath, O_RDONLY);
    int length = CountBlocks(fd);
    close(fd);
    return length;
}

int CountBlocks(int fd)
{
    struct stat st;
    off_t file_size;
    
    if (fstat(fd, &st) != 0){
        printf("fstat error\r\n");
    }

    file_size = st.st_size;
    if (file_size == -1) {
    /* Handle error */
    }

    int length = (int) file_size;
    return (length/64) - 1;
}

// gives block order in indices and returns number of blocks found
int OrderedBlockIndices(char *filepath, int *indices, int length)
{
    FILE *f;
    f = fopen(metafilePath, "rb");
    fseek(f, 64, SEEK_SET);

    
    int block_indices[length];
    
    for(int i = 0; i<length; i++)
    {
        char block[64];
        fread(block, 1, 64, f);
        int index = GetBlockIndex(block);
        block_indices[i] = index;
    }
    fclose(f);


    // read all indices in order to array
    int largestFound = -1;
    for(int i = 0; i < length-1; i++)
    {
        indices[i] =  0;
    }
    
    for(int i = 0; i < length; i++)
    {
        bool found = false;
        for(int j = length - 1; j>=0; j--)
        {
            if(block_indices[j] == i)
            {
                indices[i] = j;
                largestFound = i;
                found = true;
                break;
            }
        }

        if(!found)
        {
            break;   
        }
    }
    return largestFound;
}

void UPLINK_FNV()
{
    if(!UPLINK_ACTIVE)
    {
        NoActiveFile();
        return;
    }
    
    
    int block_count = CountBlocks(metafilePath);

    // Read existing indices
    int block_indices[block_count];
    
    int largestIndex = OrderedBlockIndices(metafilePath, block_indices, block_count);

    // calculate fnv hash for the blocks in order
    uint32_t hval = 0x811c9dc5;
    uint32_t fnv_32_prime = 0x01000193;
    uint64_t uint32_max = 4294967296;

    FILE *f;
    f = fopen(metafilePath, "rb");

    for(int i = 0; i<=largestIndex; i++)
    {
        char block[64];
        fseek(f, 64 * block_indices[i] + 64, SEEK_SET);
        fread(block, 1, 64, f);

        int block_length = GetBlockLength(block);
        for(int j=3; j<block_length+3; j++)
        {
            hval = hval ^ block[j];
            hval = (hval * fnv_32_prime) % uint32_max; 
        }
    }
    fclose(f);

    char msg[4];
    memcpy(msg, &hval, 4);

    sendPacket(FILETRANSFER, 4, msg, 4);

}

uint8_t DELETE_UPLINK_FILE()
{
    if(!UPLINK_ACTIVE)
    {
        NoActiveFile();
        return 1;
    }
    char fp[128];
    memset(fp, 0, 128);
    CreateFilePath(fp, Uplink_file_name, "");

    printf("delete %s\r\n", fp);

    return remove(fp);
    
}

void PRINT_METAFILE()
{
    if(!UPLINK_ACTIVE)
    {
        NoActiveFile();
        return;
    }

    int f = open(metafilePath, O_RDONLY);
    
    int length = CountBlocks(f);
    length++;
    printf("Length %d lines\r\n", length);

    for(int i = 0; i<length; i++)
    {
        char buf[64];
        read(f, buf, 64);
        int index = GetBlockIndex(buf);
        int blockLength = GetBlockLength(buf);
        printf("%d %d: %s\r\n", index, blockLength, buf+3);
    }
    close(f);
}

void PRINT_FILE()
{
    PRINT_METAFILE();
    return;
    if(!UPLINK_ACTIVE)
    {
        NoActiveFile();
        return;
    }

    char fp[128];
    memset(fp, 0, 128);
    CreateFilePath(fp, Uplink_file_name, "");
    int linelength = 55;
    int f = open(fp, O_RDONLY);
    
    int length = CountBlocks(f);

    printf("Length %d lines\r\n", length);

    for(int i = 0; i<length; i++)
    {
        char buf[linelength];
        read(f, buf, 64);
        _pc.write(buf, linelength);
        ThisThread::sleep_for(10ms);
        feed_the_dog();
    }
    close(f);
}

void FINALIZE_UPLINK()
{
    if(!UPLINK_ACTIVE)
    {
        NoActiveFile();
        return;
    }

    char fp[128];
    memset(fp, 0, 128);
    CreateFilePath(fp, Uplink_file_name, "");

    printf("Filepath %s\r\n", fp);

    // load the saved file parameters
    char metaFileParams[64];
    ReadMetaLine(metaFileParams, metafilePath);
    char *block_count_string = strtok(metaFileParams, " ");
    char *filename = strtok(NULL, " ");

    int metadataBlockCount = str2int(block_count_string);

    int block_count = CountBlocks(metafilePath);
    int indices[block_count];

    int highestIndex = OrderedBlockIndices(metafilePath, indices, block_count);

    for(int i = 0; i<highestIndex+1; i++)
    {
        printf("%d: %d\r\n", i, indices[i]);
    }

    printf("That's all values\r\n");

    if(highestIndex < metadataBlockCount)
    {
        // file not ready, do not proceed
        //return;
    }


    feed_the_dog();

    // save the blocks to os    

    FILE *infile;
    infile = fopen(metafilePath, "rb");

    FILE *outfile;
    outfile = fopen(fp, "wb");


    for(int i = 0; i<highestIndex+1; i++)
    {
        printf("%d: block num %d\r\n", i, indices[i]);
        feed_the_dog();
        char block[64];
        fseek(infile, ((indices[i]*64)+64), SEEK_SET);
        fread(block, 1, 64, infile);
        int length = GetBlockLength(block);
        fwrite(block, 1, 64, outfile);
    }

    // clean up
    fclose(infile);
    fclose(outfile);
    uint8_t res = remove(metafilePath);

}


void UPLINK(char *params, int data_len)
{
    // if the file is not known, do nothing. Error handling is done through the status command
    if(!UPLINK_ACTIVE)
    {
        return;
    }


    uint16_t block_number;
    block_number = params[0] | (uint16_t)params[1] << 8;
    
    char block[UPBLOCK_MAX_LENGTH];
    memset(block, 0, UPBLOCK_MAX_LENGTH);
    memcpy(block, params+2, data_len-2);

    int now = ms_since_boot();
    SaveBlock(block, block_number, data_len - 2);
    //printf("Received block %d\r\n: %d", block_number, ms_since_boot() - now);
}

int INITIATE_UPLINK(char *params)
{
    mkdir(UPLINK_FOLDER, 0777);

    char paramscopy[strlen(params)+1];
    strcpy(paramscopy, params);

    char *block_count_string = strtok(paramscopy, " ");
    char *filename = strtok(NULL, " ");

    int block_count = str2int(block_count_string);

    UPLINK_ACTIVE = 1;

    char _metafilePath[128];
    memset(_metafilePath, 0, 128);
    CreateFilePath(_metafilePath, filename, METAFILE_EXTENSION);
    printf("Filepath: %s\r\n", _metafilePath);


    if(File_exists(_metafilePath))
    {
        // read metafile and compare it to new params
        char metaFileParams[64];
        ReadMetaLine(metaFileParams, _metafilePath);

        if(strcmp(metaFileParams, params))
        {
            if(remove(_metafilePath)) return 1;
            
            CreateUplinkFiles(_metafilePath, filename, block_count);
        }
    }else
    {
        // start new
        CreateUplinkFiles(_metafilePath, filename, block_count);
    }

    
    UPLINK_ACTIVE = 1;
    strncpy(Uplink_file_name, filename, strlen(filename)<63?strlen(filename):64);
    strncpy(metafilePath, _metafilePath, 128);

    return 0;
}
