#ifndef _SHELL_COMMAND_REGISTRATION_
#define _SHELL_COMMAND_REGISTRATION_

/* ************************************************************************** */
// assemble shell commands to be registered
/* [[[cog
import codegen as code
from pathlib import Path

files = utils.all_files('src', '*.c')
files.remove('src/os/shell/shell_builtin_commands.c')
search_pattern = r'(?<=void ).*?(?=\(int argc, char \*\*argv\) \{)'

builtins = utils.search('src/os/shell/shell_builtin_commands.c', search_pattern)
commands = utils.search(files, search_pattern)

builtins = sorted(builtins)
commands = sorted(commands)

strings = builtins + commands

body = []
line = body.append

line('// extern definitions')
for s in strings:
    line(f'extern void {s}(int argc, char **argv);')

line('')
line('void register_all_shell_commands(void) {')
line('    // builtins:')
for s in builtins:
    line(f'    shell_register_command({s}, "{s[3:]}");')
line('')
line('    // application commands:')
for s in commands:
    line(f'    shell_register_command({s}, "{s[3:]}");')
line('}')

code.SourceFile(
    name = Path(Path(cog.inFile).parent, 'all_shell_commands.c'),
    includes = ['"shell_command_processor.h"'],
    contents = [body]
).write()

header = code.HeaderFile(
    name = Path(Path(cog.inFile).parent, 'all_shell_commands.h'),
    contents = ['extern void register_all_shell_commands(void);']
).write()

cog.outl()
cog.outl('#define REGISTRATION_GENERATED_SUCCESSFULLY')
cog.outl()
]]] */
/* [[[end]]] */

#ifdef REGISTRATION_GENERATED_SUCCESSFULLY
#include "all_shell_commands.h"
#endif

#endif // _SHELL_COMMAND_REGISTRATION_