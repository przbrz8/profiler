#define CBC_IMPLEMENTATION
#define CBC_STRIP_PREFIX
#include "cbc.h"

int main(int argc, char** argv)
{
    auto_rebuild(argc, argv);
    Cmd cmd = {0};
    cc(&cmd);
    cc_flags(&cmd);
    cc_output(&cmd, "profiler");
    cc_inputs(&cmd, "src/main.c", "src/profiler.c");
    cmd_run(&cmd);
    return 0;
}

