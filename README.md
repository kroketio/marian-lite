Marian-lite
======

Fork of [browsermt/marian-dev](https://github.com/browsermt/marian-dev) made for [kotki](https://github.com/kroketio/kotki) and [maemo-leste](https://maemo-leste.github.io/).

- Easier/faster to compile
- Easier to package/distribute

### Changes

- Rewritten and simplified CMake build system
- Use system dependencies where possible (removed *almost* all vendored libs)
- Removed CUDA, Intel MKL in favor of OpenBLAS
- Removed code related to the training/creation of models
- Removed examples, unit tests
- Removed support for Apple, Microsoft, WASM
- 100% FOSS

### Requirements

Dependencies that marian-lite links against:

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
- `VENDORED_LIBS` - auto-download dependencies

Example:

```bash
cmake -DSTATIC=OFF -DSHARED=ON -Bbuild .
make -Cbuild -j6
sudo make -Cbuild install  # install into /usr/local/...
```

Linking against marian-lite in another (CMake) program:

```cmake
find_package(marian-lite REQUIRED)
target_link_libraries(my_target PRIVATE marian-lite::marian-lite-SHARED)  # or '-STATIC'
```

# Credits

See [browsermt/marian-dev](https://github.com/browsermt/marian-dev) for more information.

## Todo

- replace CLI11 with a system lib (some changes were made by the marian people). Possibly look to get rid of CLI11 completely.

## License

MIT
