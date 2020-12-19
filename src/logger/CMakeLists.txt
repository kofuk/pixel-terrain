# SPDX-License-Identifier: MIT

set(LOGGER_SRCS logger.cc pretty_printer.cc)
add_library(logger STATIC ${LOGGER_SRCS})
