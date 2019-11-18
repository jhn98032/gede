/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__CONFIG_H
#define FILE__CONFIG_H

#define PROJECT_CONFIG_FILENAME   "gede2.ini"

#define GLOBAL_CONFIG_DIR        ".config/gede2"

#define PROJECT_GLOBAL_CONFIG_FILENAME "proj_gede2.ini"

#define GLOBAL_CONFIG_FILENAME    "gede2.ini"

#define GDB_LOG_FILE  "gede_gdb_log.txt"

// etags command and argument to use to get list of tags
#define ETAGS_CMD1     "ctags"    // Used on Linux
#define ETAGS_CMD2     "exctags"  // Used on freebsd
#define ETAGS_ARGS    " -f - --excmd=number --fields=+nmsSk"


// Max number of recently used goto locations to save
#define MAX_GOTO_RUI_COUNT  10

// Width of items in the GoTo list widget.
#define GOTO_LISTWIDGET_ITEM_WIDTH  240


// Expand all classes if the total number of members is below this number
#define CLASS_LIST_AUTO_EXPAND_COUNT 40

// Which fileextension does rust use?
#define RUST_FILE_EXTENSION  ".rs"

// Which fileextension does go use?
#define GOLANG_FILE_EXTENSION  ".go"

#endif // FILE__CONFIG_H


