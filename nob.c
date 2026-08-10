#define NOB_IMPLEMENTATION
#include "nob.h"

#include <dirent.h>
#include <string.h>

const char* file_extension(const char* path) {
    const char* slash = strrchr(path, '/');
    const char* dot = strrchr(path, '.');
    if (!dot || (slash && dot < slash) || dot == path) return NULL;
    return dot + 1;
}

const char* filename_only(char* _path) {
    size_t string_len = strlen(_path);
    char path[string_len];
    strcpy(path, _path);

    char* slash = strrchr(path, '/');
    slash++;

    char* dot = strrchr(slash, '.');
    *dot = '\0';
    return slash;
}

typedef struct {
    char** items;
    size_t count;
    size_t capacity;
} Files;

Files find_all_files() {
    Files f = {0};
    DIR *d;
    struct dirent *dir;
    d = opendir("build");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_name[0] == '.') { continue; }

            String_Builder sb = {0};
            sb_append_cstr(&sb, "build/");
            sb_append_cstr(&sb, dir->d_name);
            sb_append_null(&sb);
            nob_da_append(&f, sb.items);
        }

        closedir(d);
    }

    return f;
}

bool walker(Nob_Walk_Entry entry) {
    const char* ext = file_extension(entry.path);
    if (!ext || strcmp(ext, "c") != 0) { return true; }

    String_Builder objName = {0};
    sb_append_cstr(&objName, "build/");
    sb_append_cstr(&objName, filename_only(entry.path));
    sb_append_cstr(&objName, ".o");
    sb_append_null(&objName);

    Cmd cmd = {0};
    cmd_append(&cmd, "cc");
    cmd_append(&cmd, "-g");
    cmd_append(&cmd, "-std=c99");
    cmd_append(&cmd, "-Wfatal-errors");
    cmd_append(&cmd, "-c");
    cmd_append(&cmd, entry.path);
    cmd_append(&cmd, "-o");
    cmd_append(&cmd, objName.items);

    return cmd_run(&cmd);
}

int compile_exe() {
    Cmd cmd = {0};
    cmd_append(&cmd, "cc");
    cmd_append(&cmd, "-g");
    cmd_append(&cmd, "-std=c99");
    cmd_append(&cmd, "-Wfatal-errors");

    Files files = find_all_files();
    cmd_extend(&cmd, &files);
    cmd_append(&cmd, "-o", "clox");

    cmd_run(&cmd);
    return 0;
}

int main(int argc, char** argv) {
    GO_REBUILD_URSELF(argc, argv);

    mkdir_if_not_exists("build/");

    if (!walk_dir("src/", walker)) { return 1; }
    if (compile_exe() != 0) { return 1; }

    return 0;
}
