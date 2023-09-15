# Setting up CFB

## Pre-Build

The easiest and fastest way to get started is simply to download the artifacts from the Github Actions build workflow. They can be found [here](https://github.com/hugsy/CFB/actions/workflows/build.yml).

## Build

Building CFB requires only [`cmake`](), and the Windows [SDK]() and [WDK](). In a developer prompt:

```powershell
cd \path\to\CFB.git
mkdir build; cd build
cmake .. -DCFB_BUILD_TOOLS:BOOL=ON
cmake --build . --verbose --parallel $env:NUMBER_OF_PROCESSORS --config $config
cd ..
```

Where `$config` can be either `RelWithDebInfo` (normal use) or `Debug` for debugging.

To isolate only the useful binaries produced from the compilation stage into a single folder, the optional next step is to run `cmake install`.

```powershell
mkdir bins; cd build
cmake --install . --config $config --prefix ../bins --verbose
cd ..
```
