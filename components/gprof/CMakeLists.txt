# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
set(srcs
    "init.c"
    "mcount.c"
    "histogram.c"
    "save.c"
    "io.c"
    "debug.c")

set(include_dirs include)

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "${include_dirs}"
                    REQUIRES spi_flash)
