
//////////////////////////////////////////////////////
// Created by Omar El Sayyed on 4th of August 2021.
//////////////////////////////////////////////////////

#include <NSystemUtils.h>
#include <NSystem.h>
#include <NString.h>
#include <NError.h>
#include <NVector.h>
#include <NCString.h>

#include <stdlib.h>
#include <memory.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <dirent.h>
#include "../../Includes/NSystemUtils.h"

void NMain(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    NSystem.initialize(NMain, argc, argv);
    NSystem.terminate();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Memory manipulation
////////////////////////////////////////////////////////////////////////////////////////////////////

static void* nMalloc(int32_t size) {
    return malloc(size);
}

static void nFree(void* address) {
    free(address);
}

static void* nMemset(void* address, int value, int32_t length) {
    return memset(address, value, length);
}

static void* nMemcpy(void* dest, const void* src, int32_t length) {
    return memcpy(dest, src, length);
}

static void* nMemmove(void* dest, const void* src, int32_t length) {
    return memmove(dest, src, length);
}

static int32_t nMemcmp(const void* ptr1, const void* ptr2, int32_t length) {
    return memcmp(ptr1, ptr2, length);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Logging
////////////////////////////////////////////////////////////////////////////////////////////////////

#define LOG_DEFINITION(tagColor, streamDefaultColor) \
    struct NString* formattedString = NString.create(""); \
    va_list vaList; \
    va_start(vaList, format); \
    int32_t errorsStart = NError.observeErrors(); \
    NString.vAppend(formattedString, format, vaList); \
    int32_t errorsCount = NError.observeErrors() - errorsStart; \
    va_end(vaList); \
    if (!errorsCount) { \
        struct NString* coloredString = NString.replace(NString.get(formattedString), NTCOLOR(STREAM_DEFAULT), streamDefaultColor); \
        if (tag && tag[0]) { \
            printf("%s%s: %s%s%s\n", tagColor, tag, streamDefaultColor, NString.get(coloredString), NTCOLOR(RESET)); \
        } else { \
            printf("%s%s%s\n", streamDefaultColor, NString.get(coloredString), NTCOLOR(RESET)); \
        } \
        NString.destroyAndFree(coloredString); \
    } \
    NString.destroyAndFree(formattedString)

static void nLogI(const char *tag, const char* format, ...) {
    LOG_DEFINITION(NTCOLOR(STREAM_DEFAULT_STRONG), NTCOLOR(RESET));
}

static void nLogW(const char *tag, const char* format, ...) {
    LOG_DEFINITION(NTCOLOR(WARNING_STRONG), NTCOLOR(WARNING));
}

static void nLogE(const char *tag, const char* format, ...) {
    LOG_DEFINITION(NTCOLOR(ERROR_STRONG), NTCOLOR(ERROR));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// File system
////////////////////////////////////////////////////////////////////////////////////////////////////

static boolean directoryEntryExists(const char* path, boolean isAsset) {
    struct stat status;
    return stat(path, &status) == 0;
}

static int32_t getDirectoryEntryType(const char* path, boolean isAsset) {
    struct stat status;
    if (stat(path, &status)) return -1;

    if      (S_ISREG (status.st_mode)) { return NDirectoryEntryType.REGULAR_FILE      ; }
    else if (S_ISDIR (status.st_mode)) { return NDirectoryEntryType.DIRECTORY         ; }
    else if (S_ISBLK (status.st_mode)) { return NDirectoryEntryType.BLOCK_DEVICE      ; }
    else if (S_ISFIFO(status.st_mode)) { return NDirectoryEntryType.NAMED_PIPE        ; }
    else if (S_ISSOCK(status.st_mode)) { return NDirectoryEntryType.UNIX_DOMAIN_SOCKET; }
    else if (S_ISCHR (status.st_mode)) { return NDirectoryEntryType.CHARACTER_DEVICE  ; }
    else if (S_ISLNK (status.st_mode)) { return NDirectoryEntryType.SYMBOLIC_LINK     ; }
    else                               { return NDirectoryEntryType.UNKNOWN           ; }
}

// Returns a pointer to full path. Remember to free it,
static char* getFullPath(const char* path) {
#if NPROFILE_MEMORY==0
    return realpath(path, 0);
#else
    char* unprofiledPath = realpath(path, 0);
    char* profiledPath = NCString.clone(unprofiledPath);
    nFree(unprofiledPath);
    return profiledPath;
#endif
}

// Returns file size if possible, -1 otherwise,
static uint32_t getFileSize(const char* filePath, boolean isAsset) {
    struct stat fileStatus;
    if (stat(filePath, &fileStatus) != 0) {
        NERROR("NSystemUtils.getFileSize()", "%sCouldn't get the size%s of %s%s%s. File %sdoesn't exist%s.", NTCOLOR(HIGHLIGHT), NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), filePath, NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), NTCOLOR(STREAM_DEFAULT));
        return -1;
    }
    return (uint32_t) fileStatus.st_size;

    // Could also use fseek (for cross compatibility),
    //    FILE *file = fopen(filePath, "rb");
    //    fseek(file, 0, SEEK_END);
    //    long fileSize = ftell(file);
    //    fseek(file, 0, SEEK_SET);  // Same as rewind(f).
    //
    //    char* code = NSystemUtils.malloc(fileSize + 1);
    //    fread(code, 1, fileSize, file);
    //    fclose(file);
}

static struct NVector* listDirectoryEntries(const char* directoryPath, boolean isAsset) {

    // If you run into compatibility problems, see: https://stackoverflow.com/a/29094555/1942069

    // Open directory,
    DIR *directory = opendir(directoryPath);
    if (!directory) {
        NERROR("NSystemUtils.listDirectoryEntries()", "%sCouldn't list contents%s of directory %s%s%s.", NTCOLOR(HIGHLIGHT),NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), directoryPath, NTCOLOR(STREAM_DEFAULT));
        return 0;
    }

    // Read contents,
    struct NVector *directoryContents = NVector.create(0, sizeof(struct NDirectoryEntry));
    struct dirent *directoryEntry;

    while ((directoryEntry = readdir(directory)) != 0) {
        struct NDirectoryEntry* currentDirectoryEntry = NVector.emplaceBack(directoryContents);
        currentDirectoryEntry->name = NCString.clone(directoryEntry->d_name);

        switch(directoryEntry->d_type) {
            case DT_FIFO: currentDirectoryEntry->type = NDirectoryEntryType.NAMED_PIPE        ; break;
            case DT_CHR : currentDirectoryEntry->type = NDirectoryEntryType.CHARACTER_DEVICE  ; break;
            case DT_DIR : currentDirectoryEntry->type = NDirectoryEntryType.DIRECTORY         ; break;
            case DT_BLK : currentDirectoryEntry->type = NDirectoryEntryType.BLOCK_DEVICE      ; break;
            case DT_REG : currentDirectoryEntry->type = NDirectoryEntryType.REGULAR_FILE      ; break;
            case DT_LNK : currentDirectoryEntry->type = NDirectoryEntryType.SYMBOLIC_LINK     ; break;
            case DT_SOCK: currentDirectoryEntry->type = NDirectoryEntryType.UNIX_DOMAIN_SOCKET; break;
            default     : currentDirectoryEntry->type = NDirectoryEntryType.UNKNOWN           ; break;
        }
    }

    closedir(directory);
    return directoryContents;
}

static void destroyAndFreeDirectoryEntryVector(struct NVector* directoryEntryVector) {
    for (int32_t i=NVector.size(directoryEntryVector)-1; i>=0; i--) {
        struct NDirectoryEntry* directoryEntry = NVector.get(directoryEntryVector, i);
        NFREE((void *) directoryEntry->name, "NSystemUtils.destroyAndFreeDirectoryEntryVector() directoryEntry->name");
    }
    NVector.destroyAndFree(directoryEntryVector);
}

// Returns read bytes count, -1 otherwise,
static uint32_t readFromFile(const char* filePath, boolean isAsset, uint32_t offsetInFile, uint32_t maxReadSize, void* output) {

    struct stat fileStatus;
    if (stat(filePath, &fileStatus) != 0) {
        NERROR("NSystemUtils.readFromFile()", "Couldn't read file: %s%s%s. File %sdoesn't exist%s.", NTCOLOR(HIGHLIGHT), filePath, NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), NTCOLOR(STREAM_DEFAULT));
        return -1;
    }

    // Force upper limit of read size,
    uint32_t readSize = fileStatus.st_size - offsetInFile;
    if (maxReadSize && (readSize > maxReadSize)) readSize = maxReadSize;

    // Open the file,
    FILE *fileHandle = fopen(filePath, "rb");
    if (!fileHandle) {
        NERROR("NSystemUtils.readFromFile()", "%sCouldn't read%s file %s%s%s.", NTCOLOR(HIGHLIGHT), NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), filePath, NTCOLOR(STREAM_DEFAULT));
        return -1;
    }

    // Forward to the offset,
    if (offsetInFile) {
        if (fseek(fileHandle, offsetInFile, SEEK_SET)) {
            NERROR("NSystemUtils.readFromFile()", "%sCouldn't seek%s to offset %s%ld%s in file %s%s%s.", NTCOLOR(HIGHLIGHT), NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), (int64_t) readSize, NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), filePath, NTCOLOR(STREAM_DEFAULT));
            return -1;
        }
    }

    // Read,
    uint32_t readBytesCount = fread(output, 1, readSize, fileHandle);
    fclose(fileHandle);

    return readBytesCount;
}

// Returns success,
static boolean writeToFile(const char* filePath, const void *data, uint32_t sizeBytes, boolean append) {

    // Open file,
    FILE *fileHandle;
    if (append) {
        fileHandle = fopen(filePath, "ab");
        if (!fileHandle) {
            NERROR("NSystemUtils.writeToFile()", "%sCouldn't open%s file for appending: %s%s%s.", NTCOLOR(HIGHLIGHT), NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), filePath, NTCOLOR(STREAM_DEFAULT));
            return False;
        }
    } else {
        fileHandle = fopen(filePath, "wb");
        if (!fileHandle) {
            NERROR("NSystemUtils.writeToFile()", "%sCouldn't open%s file for writing: %s%s%s.", NTCOLOR(HIGHLIGHT), NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), filePath, NTCOLOR(STREAM_DEFAULT));
            return False;
        }
    }

    // Write data to the file,
    uint32_t writtenBytesCount = fwrite(data, 1, sizeBytes, fileHandle);
    fclose(fileHandle);

    return writtenBytesCount;
}

// Returns success,
static boolean makeDirectory(const char* directoryPath) {

    if (mkdir(directoryPath, 0777)) {
        NERROR("NSystemUtils.writeToFile()", "%sCouldn't make directory%s: %s%s%s.", NTCOLOR(HIGHLIGHT), NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), directoryPath, NTCOLOR(STREAM_DEFAULT));
        return False;
    }

    return True;
}

const struct NDirectoryEntryType NDirectoryEntryType = {
    .UNKNOWN = 0,
    .REGULAR_FILE = 1,
    .DIRECTORY = 2,
    .BLOCK_DEVICE = 3,
    .NAMED_PIPE = 4,
    .UNIX_DOMAIN_SOCKET = 5,
    .CHARACTER_DEVICE = 6,
    .SYMBOLIC_LINK = 7
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////////////////////////

static void getTime(int64_t* outTimeSeconds, int64_t* outTimeNanos) {
    struct timespec time;
    timespec_get(&time, TIME_UTC);
    *outTimeSeconds = time.tv_sec;
    *outTimeNanos = time.tv_nsec;
}

static boolean isNaN(double value) { return isnan(value); }
static boolean isInf(double value) { return isinf(value); }

const struct NSystemUtils_Interface NSystemUtils = {
    .malloc = nMalloc,
    .free = nFree,
    .memset = nMemset,
    .memcpy = nMemcpy,
    .memmove = nMemmove,
    .memcmp = nMemcmp,
    .logI = nLogI,
    .logW = nLogW,
    .logE = nLogE,
    .directoryEntryExists = directoryEntryExists,
    .getDirectoryEntryType = getDirectoryEntryType,
    .getFullPath = getFullPath,
    .getFileSize = getFileSize,
    .listDirectoryEntries = listDirectoryEntries,
    .destroyAndFreeDirectoryEntryVector = destroyAndFreeDirectoryEntryVector,
    .readFromFile = readFromFile,
    .writeToFile = writeToFile,
    .makeDirectory = makeDirectory,
    .getTime = getTime,
    .isNaN = isNaN,
    .isInf = isInf
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Terminal colors
////////////////////////////////////////////////////////////////////////////////////////////////////

const struct NTerminalColor NTerminalColor = {
    .RESET = "\033[0m",

    .BLACK   = "\033[0;30m",
    .RED     = "\033[0;31m",
    .GREEN   = "\033[0;32m",
    .YELLOW  = "\033[0;33m",
    .BLUE    = "\033[0;34m",
    .MAGENTA = "\033[0;35m",
    .CYAN    = "\033[0;36m",
    .WHITE   = "\033[0;37m",

    .BLACK_BOLD   = "\033[1;30m",
    .RED_BOLD     = "\033[1;31m",
    .GREEN_BOLD   = "\033[1;32m",
    .YELLOW_BOLD  = "\033[1;33m",
    .BLUE_BOLD    = "\033[1;34m",
    .MAGENTA_BOLD = "\033[1;35m",
    .CYAN_BOLD    = "\033[1;36m",
    .WHITE_BOLD   = "\033[1;37m",

    .BLACK_UNDERLINED   = "\033[4;30m",
    .RED_UNDERLINED     = "\033[4;31m",
    .GREEN_UNDERLINED   = "\033[4;32m",
    .YELLOW_UNDERLINED  = "\033[4;33m",
    .BLUE_UNDERLINED    = "\033[4;34m",
    .MAGENTA_UNDERLINED = "\033[4;35m",
    .CYAN_UNDERLINED    = "\033[4;36m",
    .WHITE_UNDERLINED   = "\033[4;37m",

    .BLACK_BACKGROUND   = "\033[40m",
    .RED_BACKGROUND     = "\033[41m",
    .GREEN_BACKGROUND   = "\033[42m",
    .YELLOW_BACKGROUND  = "\033[43m",
    .BLUE_BACKGROUND    = "\033[44m",
    .MAGENTA_BACKGROUND = "\033[45m",
    .CYAN_BACKGROUND    = "\033[46m",
    .WHITE_BACKGROUND   = "\033[47m",

    .BLACK_BRIGHT   = "\033[0;90m",
    .RED_BRIGHT     = "\033[0;91m",
    .GREEN_BRIGHT   = "\033[0;92m",
    .YELLOW_BRIGHT  = "\033[0;93m",
    .BLUE_BRIGHT    = "\033[0;94m",
    .MAGENTA_BRIGHT = "\033[0;95m",
    .CYAN_BRIGHT    = "\033[0;96m",
    .WHITE_BRIGHT   = "\033[0;97m",

    .BLACK_BOLD_BRIGHT   = "\033[1;90m",
    .RED_BOLD_BRIGHT     = "\033[1;91m",
    .GREEN_BOLD_BRIGHT   = "\033[1;92m",
    .YELLOW_BOLD_BRIGHT  = "\033[1;93m",
    .BLUE_BOLD_BRIGHT    = "\033[1;94m",
    .MAGENTA_BOLD_BRIGHT = "\033[1;95m",
    .CYAN_BOLD_BRIGHT    = "\033[1;96m",
    .WHITE_BOLD_BRIGHT   = "\033[1;97m",

    .BLACK_BACKGROUND_BRIGHT   = "\033[0;100m",
    .RED_BACKGROUND_BRIGHT     = "\033[0;101m",
    .GREEN_BACKGROUND_BRIGHT   = "\033[0;102m",
    .YELLOW_BACKGROUND_BRIGHT  = "\033[0;103m",
    .BLUE_BACKGROUND_BRIGHT    = "\033[0;104m",
    .MAGENTA_BACKGROUND_BRIGHT = "\033[0;105m",
    .CYAN_BACKGROUND_BRIGHT    = "\033[0;106m",
    .WHITE_BACKGROUND_BRIGHT   = "\033[0;107m",

    .STREAM_DEFAULT = "\033NOMoneSD",
    .STREAM_DEFAULT_STRONG = "\033[1;97m", // WHITE_BOLD_BRIGHT
    .ERROR = "\033[0;31m",                 // RED_BOLD
    .ERROR_STRONG = "\033[1;91m",          // RED_BOLD_BRIGHT
    .WARNING = "\033[0;33m",               // YELLOW
    .WARNING_STRONG = "\033[1;93m",        // YELLOW_BOLD_BRIGHT
    .HIGHLIGHT = "\033[1;93m",             // YELLOW_BOLD_BRIGHT
    .DANGER = "\033[1;91m"                 // RED_BOLD_BRIGHT
};
