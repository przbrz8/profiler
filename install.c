#define CBC_IMPLEMENTATION
#define CBC_STRIP_PREFIX
#include "cbc.h"

int main(int argc, char** argv)
{
    auto_rebuild(argc, argv);
    Cmd cmd = {0};
    cmd_append(&cmd, "mkdir -p build");
    cmd_run(&cmd);
    cc(&cmd);
    cc_output(&cmd, "build/profiler.o");
    cc_inputs(&cmd, "-c src/profiler.c");
    cmd_run(&cmd);
    cmd_append(&cmd, "ar rcs build/libprof.a build/profiler.o");
    cmd_run(&cmd);
    cmd_append(&cmd, "sudo cp build/libprof.a /usr/lib/");
    cmd_run(&cmd);
    cmd_append(&cmd, "sudo cp src/profiler.h /usr/include/");
    cmd_run(&cmd);
    cmd_append(&cmd, "sudo ldconfig");
    cmd_run(&cmd);
    cmd_append(&cmd, "rm -r build");
    cmd_run(&cmd);
    return 0;
}

