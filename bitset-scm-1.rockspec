package = 'bitset'
version = 'scm-1'
source  = {
    url    = 'git://github.com/oleggator/bitset.git',
    branch = 'master',
}
description = {
    summary  = "Bitset for Tarantool",
    homepage = 'https://github.com/oleggator/bitset/',
    license  = 'BSD',
}
dependencies = {
    'lua >= 5.1',
    'checks >= 3.0.1',
}
external_dependencies = {
    TARANTOOL = {
        header = "tarantool/module.h"
    },
}
build = {
    type = 'cmake',

    variables = {
        version = 'scm-1',
        TARANTOOL_DIR = '$(TARANTOOL_DIR)',
        TARANTOOL_INSTALL_LIBDIR = '$(LIBDIR)',
        TARANTOOL_INSTALL_LUADIR = '$(LUADIR)',
    }
}

-- vim: syntax=lua