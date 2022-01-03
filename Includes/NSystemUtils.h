
//////////////////////////////////////////////////////
// Created by Omar El Sayyed on 4th of August 2021.
//////////////////////////////////////////////////////

#pragma once

#include <NTypes.h>
#include <NMemoryProfiler.h>

#ifndef NVERBOSE
    #define NVERBOSE 1
#endif
#if NVERBOSE==1
    #define NLOGI(tag, format, ...) NSystemUtils.logI(tag, format, ##__VA_ARGS__)
    #define NLOGW(tag, format, ...) NSystemUtils.logW(tag, format, ##__VA_ARGS__)
    #define NLOGE(tag, format, ...) NSystemUtils.logE(tag, format, ##__VA_ARGS__)
#else
    #define NLOGI(tag, format, ...)
    #define NLOGW(tag, format, ...)
    #define NLOGE(tag, format, ...)
#endif

// NPROFILE_MEMORY values:
//   0: None.
//   1: Basic mode (default), track memory leaks.
//   2: Doesn't delete allocation tracking data, so we can generate more data when the app finishes,
//      but the app's memory usage grows steadily.
//   3: Detailed info. Aggregation data always present. Not meant for detecting leaks and such. It's
//      for analyzing app's memory usage performance.

#ifndef NPROFILE_MEMORY
    #define NPROFILE_MEMORY 1
#endif
#if NPROFILE_MEMORY > 0
    #define NMALLOC(size   , id) NMemoryProfiler_malloc(size   , id)
    #define   NFREE(address, id) NMemoryProfiler_free  (address, id)
#else
    #define NMALLOC(size   , id) NSystemUtils.malloc(size   )
    #define   NFREE(address, id) NSystemUtils.free  (address)
#endif

#define NTCOLOR(color) NTerminalColor.color

struct NVector;

struct NDirectoryEntry {
    int32_t type;
    char* name;
};

struct NSystemUtils_Interface {

    // Memory management,
    void* (*malloc)(int32_t size);
    void (*free)(void* address);
    void* (*memset)(void* address, int value, int32_t length); // Returns address.
    void* (*memcpy)(void* dest, const void* src, int32_t length); // Returns destination.

    // Log,
    void (*logI)(const char* tag, const char* format, ...);
    void (*logW)(const char* tag, const char* format, ...);
    void (*logE)(const char* tag, const char* format, ...);

    // File system,
    boolean (*directoryEntryExists)(const char* path, boolean isAsset);
    int32_t (*getDirectoryEntryType)(const char* path, boolean isAsset); // Returns type, -1 if not possible.
    char* (*getFullPath)(const char* path); // Returns a pointer to full path. Remember to free it.
    uint32_t (*getFileSize)(const char* filePath, boolean isAsset); // Returns file size if possible, -1 otherwise.
    struct NVector* (*listDirectoryEntries)(const char* directoryPath, boolean isAsset);
    void (*destroyAndFreeDirectoryEntryVector)(struct NVector* directoryEntryVector);
    uint32_t (*readFromFile)(const char* filePath, boolean isAsset, uint32_t offsetInFile, uint32_t maxReadSize, void* output); // Returns read bytes count, -1 otherwise.
    boolean (*writeToFile)(const char* filePath, const void *data, uint32_t sizeBytes, boolean append); // Returns success.
    boolean (*makeDirectory)(const char* directoryPath); // Returns success.

    // Misc,
    void (*getTime)(int64_t* outTimeSeconds, int64_t* outTimeNanos);
    boolean (*isNaN)(double value);
    boolean (*isInf)(double value);
};

extern const struct NSystemUtils_Interface NSystemUtils;

struct NDirectoryEntryType {
    const int32_t UNKNOWN;
    const int32_t NAMED_PIPE;
    const int32_t CHARACTER_DEVICE;
    const int32_t DIRECTORY;
    const int32_t BLOCK_DEVICE;
    const int32_t REGULAR_FILE;
    const int32_t SYMBOLIC_LINK;
    const int32_t UNIX_DOMAIN_SOCKET;
};
extern const struct NDirectoryEntryType NDirectoryEntryType;

struct NTerminalColor {
    // Thanks to this answer: https://stackoverflow.com/a/51944613/1942069

    // Color end string, color reset,
    const char* const RESET;

    // Regular Colors. Normal color; no bold; background color etc;
    const char* const BLACK;
    const char* const RED;
    const char* const GREEN;
    const char* const YELLOW;
    const char* const BLUE;
    const char* const MAGENTA;
    const char* const CYAN;
    const char* const WHITE;

    // Bold;
    const char* const BLACK_BOLD;
    const char* const RED_BOLD;
    const char* const GREEN_BOLD;
    const char* const YELLOW_BOLD;
    const char* const BLUE_BOLD;
    const char* const MAGENTA_BOLD;
    const char* const CYAN_BOLD;
    const char* const WHITE_BOLD;

    // Underline;
    const char* const BLACK_UNDERLINED;
    const char* const RED_UNDERLINED;
    const char* const GREEN_UNDERLINED;
    const char* const YELLOW_UNDERLINED;
    const char* const BLUE_UNDERLINED;
    const char* const MAGENTA_UNDERLINED;
    const char* const CYAN_UNDERLINED;
    const char* const WHITE_UNDERLINED;

    // Background;
    const char* const BLACK_BACKGROUND;
    const char* const RED_BACKGROUND;
    const char* const GREEN_BACKGROUND;
    const char* const YELLOW_BACKGROUND;
    const char* const BLUE_BACKGROUND;
    const char* const MAGENTA_BACKGROUND;
    const char* const CYAN_BACKGROUND;
    const char* const WHITE_BACKGROUND;

    // High Intensity;
    const char* const BLACK_BRIGHT;
    const char* const RED_BRIGHT;
    const char* const GREEN_BRIGHT;
    const char* const YELLOW_BRIGHT;
    const char* const BLUE_BRIGHT;
    const char* const MAGENTA_BRIGHT;
    const char* const CYAN_BRIGHT;
    const char* const WHITE_BRIGHT;

    // Bold High Intensity;
    const char* const BLACK_BOLD_BRIGHT;
    const char* const RED_BOLD_BRIGHT;
    const char* const GREEN_BOLD_BRIGHT;
    const char* const YELLOW_BOLD_BRIGHT;
    const char* const BLUE_BOLD_BRIGHT;
    const char* const MAGENTA_BOLD_BRIGHT;
    const char* const CYAN_BOLD_BRIGHT;
    const char* const WHITE_BOLD_BRIGHT;

    // High Intensity backgrounds;
    const char* const BLACK_BACKGROUND_BRIGHT;
    const char* const RED_BACKGROUND_BRIGHT;
    const char* const GREEN_BACKGROUND_BRIGHT;
    const char* const YELLOW_BACKGROUND_BRIGHT;
    const char* const BLUE_BACKGROUND_BRIGHT;
    const char* const MAGENTA_BACKGROUND_BRIGHT;
    const char* const CYAN_BACKGROUND_BRIGHT;
    const char* const WHITE_BACKGROUND_BRIGHT;

    // Symantec colors,
    const char* const STREAM_DEFAULT;
    const char* const STREAM_DEFAULT_STRONG;
    const char* const ERROR;
    const char* const ERROR_STRONG;
    const char* const WARNING;
    const char* const WARNING_STRONG;
    const char* const HIGHLIGHT;
    const char* const DANGER;
};
extern const struct NTerminalColor NTerminalColor;
