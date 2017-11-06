## Grimoire {#mainpage}
General lib, made to be used as a submodule
https://rawgit.com/bfs15/Grimoire/doc/doc/annotated.html

`git submodule add https://github.com/bfs15/Grimoire.git`
this will add a folder ./Grimoire, it will be its own repository, see the git doc "submodules" for more information.

To use it Add `-I./Grimoire/include` to the compiler and `#include "varray.hpp"` the desired header to your code.

To use a project with a submodule, update it with
`git submodule update --recursive`
