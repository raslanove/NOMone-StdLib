
#include <NSystem.h>
#include <NSystemUtils.h>
#include <NMemoryProfiler.h>

static void initialize(void (*nMain)(int argc, char *argv[]), int argc, char *argv[]) {
    #if NPROFILE_MEMORY > 0
        NMemoryProfiler_initialize();
    #endif
    if (nMain) nMain(argc, argv);
}

static void terminate() {
    #if NPROFILE_MEMORY > 0
        NMemoryProfiler_logOnExitReport();
    #endif
}

const struct NSystem_Interface NSystem = {
    .initialize = initialize,
    .terminate = terminate
};
