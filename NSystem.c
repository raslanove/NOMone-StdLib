
#include <NSystem.h>
#include <NSystemUtils.h>

static void initialize(void (*nMain)()) {
    if (nMain) nMain();
}

static void terminate() {
    if (NSystem_mallocCallsCount != NSystem_freeCallsCount) NLOGE("NSystem", "mallocs != frees. mallocs: %s%ld%s, frees: %s%ld%s", NTCOLOR(HIGHLIGHT), NSystem_mallocCallsCount, NTCOLOR(STREAM_DEFAULT), NTCOLOR(HIGHLIGHT), NSystem_freeCallsCount, NTCOLOR(STREAM_DEFAULT));
}

const struct NSystem_Interface NSystem = {
    .initialize = initialize,
    .terminate = terminate
};

int64_t NSystem_mallocCallsCount, NSystem_freeCallsCount;