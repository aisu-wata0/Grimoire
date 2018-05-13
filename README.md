## Grimoire {#mainpage}
General lib, made to be used as a submodule
https://rawgit.com/bfs15/Grimoire/doc/doc/annotated.html

`git submodule add https://github.com/bfs15/Grimoire.git;`
`git submodule update --recursive --init;`
Will create a folder ./Grimoire, it will be its own repository.
Second command will initialize it.

Update submodules with
`git submodule update --recursive`

To use it Add `-I./Grimoire/include` to the compiler flags and include the desired header to your code. (`#include "varray.hpp"`)

See the git doc "submodules" for more information.
