#define NOB_IMPLEMENTATION
#include "nob.h"

// Some folder paths that we use throughout the build process.
#define BUILD_FOLDER "build/"
#define SRC_FOLDER   "src/"

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) return 1;

    const char* program = nob_shift(argv, argc);

    // The working horse of nob is the Nob_Cmd structure. It's a Dynamic Array of strings which represent
    // command line that you want to execute.
    Nob_Cmd cmd = {0};

    //nob_cmd_append(&cmd, "cc", "-ggdb", "-Wall", "-Wextra", "-o", BUILD_FOLDER"main", SRC_FOLDER"main.c");
    //nob_cmd_append(&cmd, "-lraylib", "-lGL", "-lm", "-lpthread", "-ldl", "-lrt", "-lX11");

    //nob_cmd_append(&cmd, "cc", "-ggdb", "-Wall", "-Wextra", "-o", BUILD_FOLDER"cjson", SRC_FOLDER"json.c");

    nob_cmd_append(&cmd, "cc", "-ggdb", "-Wall", "-Wextra", "-o", BUILD_FOLDER"json_builder", SRC_FOLDER"json_builder.c");

    // nob_cmd_run_sync_and_reset() resets the cmd for you automatically
    if (!nob_cmd_run_sync_and_reset(&cmd)) return 1;

    if (argc > 0) {
        const char* param = nob_shift(argv, argc);
        if (strcmp(param, "run") == 0) {
            //nob_cmd_append(&cmd, "./cjson");
            nob_cmd_append(&cmd, "./json_builder");
            if (!nob_cmd_run_sync_and_reset(&cmd)) return 1;
        } else if (strcmp(param, "debug") == 0) {
            //nob_cmd_append(&cmd, "~/software/GF/gf2", BUILD_FOLDER"./cjson");
            nob_cmd_append(&cmd, "~/software/GF/gf2", BUILD_FOLDER"./json_builder");
        }
    }

    return 0;
}
