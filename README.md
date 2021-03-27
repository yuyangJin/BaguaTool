# BaguaTool


## Dependency and Build

BaguaTool is dependent on
* [Dyninst](https://github.com/dyninst/dyninst)
* [PAPI](https://bitbucket.org/icl/papi/src/master/)
* [igraph](https://github.com/igraph/igraph)
* cmake >= 3.16

```igraph``` has been integrated into baguatool as submodule. Dyninst and PAPI need user to build themselves.

The recommended way to build Dyninst and PAPI is to use [Spack](https://github.com/spack/spack)

```bash
spack install dyninst
spack install papi

# before building BaguaTool
spack load dyninst
spack load papi
```

Or for users who builds these from source, you need to specify where to find these

```bash
cmake .. -DDyninst_DIR=/path_to_your_dyninst_install_dir/lib/cmake/Dyninst -DPAPI_PREFIX=/path_to_your_papi_install_dir

# you should make sure that there is `DyninstConfig.cmake` in /path_to_your_dyninst_install_dir/lib/cmake/Dyninst
# And there is `include` `lib` in /path_to_your_papi_install_dir
```
