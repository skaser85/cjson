#define NOB_IMPLEMENTATION
#include "nob.h"

// Some folder paths that we use throughout the build process.
#define BUILD_FOLDER "build/"
#define SRC_FOLDER   "src/"

#define DEBUGGER_PATH "gf"

typedef struct {
    const char *src_name;
    const char *app_name;
    const char *alias;
} Project;

typedef struct {
    Project *items;
    size_t count;
    size_t capacity;
} Projects;

bool build_it(Project p) {
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "cc", "-ggdb", "-Wall", "-Wextra", "-o", nob_temp_sprintf(BUILD_FOLDER"%s", p.app_name), nob_temp_sprintf(SRC_FOLDER"%s", p.src_name));
    return nob_cmd_run_sync_and_reset(&cmd);
}

bool run_it(Project p) {
    Nob_Cmd cmd = {0};
    const char *path = nob_temp_sprintf("./%s%s", BUILD_FOLDER, p.app_name);
    nob_cmd_append(&cmd, path);
    return nob_cmd_run_sync_and_reset(&cmd);
}

bool debug_it(Project p) {
    Nob_Cmd cmd = {0};
    const char *app_path = nob_temp_sprintf("./%s%s", BUILD_FOLDER, p.app_name);
    nob_cmd_append(&cmd, DEBUGGER_PATH);
    nob_cmd_append(&cmd, app_path);
    return nob_cmd_run_sync_and_reset(&cmd);
}

bool build_and_run(Project p) {
    if (!build_it(p))
        return false;
    run_it(p);
    return true;
}

void add_project(Projects *projects, const char *app_name, const char *src_name, const char *alias) {
    Project *p = malloc(sizeof(Project));
    memset(p, 0, sizeof(Project));
    p->src_name = strdup(src_name);
    p->app_name = strdup(app_name);
    p->alias = strdup(alias);
    nob_da_append(projects, *p);
}

Project *get_project(Projects projects, const char *alias) {
    for (size_t i = 0; i < projects.count; ++i) {
        if (strcmp(projects.items[i].alias, alias) == 0) {
            return &projects.items[i];
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{

    NOB_GO_REBUILD_URSELF(argc, argv);
    
    Projects projects = {0};
    add_project(&projects, "json_builder", "json_builder.c", "builder");
    add_project(&projects, "json_parser", "json_parser.c", "parser");

    if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) return 1;

    const char* program = nob_shift(argv, argc);

    //nob_cmd_append(&cmd, "cc", "-ggdb", "-Wall", "-Wextra", "-o", BUILD_FOLDER"main", SRC_FOLDER"main.c");
    //nob_cmd_append(&cmd, "-lraylib", "-lGL", "-lm", "-lpthread", "-ldl", "-lrt", "-lX11");

    if (argc > 0) {
        const char* param = nob_shift(argv, argc);
        if (strcmp(param, "run") == 0) {
            if (argc == 0) {
                nob_log(NOB_ERROR, "No app name was provided to the `run` command!");
                return 1;
            }
            param = nob_shift(argv, argc);
            Project *p = get_project(projects, param);
            if (p) {
                build_and_run(*p);
            } else {
                nob_log(NOB_ERROR, "Invalid app name provided to nob! (`parser` or `builder`)");
                return 1;
            }
        } else if (strcmp(param, "build") == 0) {
            if (argc == 0) {
                nob_log(NOB_ERROR, "No app name was provided to the `build` command!");
                return 1;
            }
            param = nob_shift(argv, argc);
            Project *p = get_project(projects, param);
            if (p) {
                build_it(*p);
            } else{
                nob_log(NOB_ERROR, "Invalid app name provided to nob! (`parser` or `builder`)");
                return 1;
            }
        } else if (strcmp(param, "debug") == 0) {
            if (argc == 0) {
                nob_log(NOB_ERROR, "No app name was provided to the `debug` command!");
                return 1;
            }
            param = nob_shift(argv, argc);
            Project *p = get_project(projects, param);
            if (p) {
                debug_it(*p);
            } else {
                nob_log(NOB_ERROR, "Invalid app name provided to nob! (`parser` or `builder`)");
                return 1;
            }
        }
    } else {
        nob_log(NOB_ERROR, "No arguments were provided to nob! (`run` or `build`)");
        return 1;
    }


    return 0;
}

