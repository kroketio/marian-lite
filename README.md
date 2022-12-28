Marian-lite
======

Our fork of [browsermt/marian-dev](https://github.com/browsermt/marian-dev) made for [kotki](https://github.com/kroketio/kotki) and [maemo-leste](https://maemo-leste.github.io/).

- Easier/faster to compile
- Easier to package/distribute

### Changes

- Rewritten and simplified CMake build system
- Use system dependencies where possible (removed *almost* all vendored libs)
- Removed support for proprietary stuff like CUDA, Intel MKL
- Removed code related to the training of translation models
- Removed examples, unit tests
- Removed support for Apple, Microsoft, WASM
- 100% FOSS

### Requirements

Some dependencies that marian-lite links against:

- https://github.com/kroketio/intgemm/
- https://github.com/kroketio/pathie-cpp/
- https://github.com/kroketio/sentencepiece-browsermt/

And others:

```bash
sudo apt install -y \
    cmake \
    ccache \
    pkg-config \
    libopenblas-dev \
    libyaml-cpp-dev \
    libprotobuf-lite* \
    protobuf-compiler \
    libsqlite3-dev \
    libpcre2-dev \
    zlib1g-dev
```

### Building

Some CMake options available:

- `STATIC` - Produce static binary
- `SHARED` - Produce shared binary

Example:

```bash
cmake -DSTATIC=OFF -DSHARED=ON -Bbuild .
make -Cbuild -j6
sudo make -Cbuild install  # install into /usr/local/...
```

Linking against marian-lite in another (CMake) program:

```cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(MARIAN-LITE REQUIRED marian-lite)

message(STATUS "marian-lite libraries: ${MARIAN-LITE_LIBRARIES}")
message(STATUS "marian-lite include dirs: ${MARIAN-LITE_INCLUDE_DIRS}")

target_link_libraries(myapp PUBLIC
        ${MARIAN-LITE_LIBRARIES}
        )

target_include_directories(myapp PUBLIC
        ${MARIAN-LITE_INCLUDE_DIRS}
        )
```

# Credits

Named in honour of Marian Rejewski, a Polish mathematician and cryptologist.

Marcin Junczys-Dowmunt, Roman Grundkiewicz, Tomasz Dwojak, Hieu Hoang, Kenneth
Heafield, Tom Neckermann, Frank Seide, Ulrich Germann, Alham Fikri Aji, Nikolay
Bogoychev, André F. T. Martins, Alexandra Birch (2018). Marian: Fast Neural
Machine Translation in C++ (http://www.aclweb.org/anthology/P18-4020)

## Acknowledgements

The development of Marian received funding from the European Union's
_Horizon 2020 Research and Innovation Programme_ under grant agreements
688139 ([SUMMA](http://www.summa-project.eu); 2016-2019),
645487 ([Modern MT](http://www.modernmt.eu); 2015-2017),
644333 ([TraMOOC](http://tramooc.eu/); 2015-2017),
644402 ([HiML](http://www.himl.eu/); 2015-2017),
825303 ([Bergamot](https://browser.mt/); 2019-2021),
the Amazon Academic Research Awards program,
the World Intellectual Property Organization,
and is based upon work supported in part by the Office of the Director of
National Intelligence (ODNI), Intelligence Advanced Research Projects Activity
(IARPA), via contract #FA8650-17-C-9117.

This software contains source code provided by NVIDIA Corporation.

## Todo

- replace CLI11 with a system lib (some changes were made by the marian people). Possibly look to get rid of CLI11 completely.

## License

MIT
