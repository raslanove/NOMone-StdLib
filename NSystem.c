
#include <NSystem.h>
#include <NSystemUtils.h>
#include <NMemoryProfiler.h>

static void initialize(void (*nMain)(int argc, char *argv[]), int argc, char *argv[]) {
    NMemoryProfiler_initialize();
    if (nMain) nMain(argc, argv);
}

static void terminate() {
    NMemoryProfiler_logOnExitReport();
}

const struct NSystem_Interface NSystem = {
    .initialize = initialize,
    .terminate = terminate
};
