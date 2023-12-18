# Build CFB

## Pre-Build

The easiest and fastest way to get started is simply to download the artifacts from the Github Actions build workflow. They can be found [here](https://github.com/hugsy/CFB/actions/workflows/build.yml).

## Build

Building CFB requires only [`cmake`](https://cmake.org), and the Windows [SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/) and [WDK](https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk). In a developer prompt:

### To compile
```powershell
cd \path\to\CFB.git
mkdir build
cmake -S . -B ./build -D CFB_BUILD_TOOLS:BOOL=ON -A $platform
cmake --build ./build --verbose --parallel $env:NUMBER_OF_PROCESSORS --config $config
```

Where `$platform` can be:
  - `x64`
  - `arm64`

`win32` may work but is totally untested. Feedback welcome.

### To install 

After building:

```powershell
mkdir artifact
cmake --install ./build --config $config --prefix ./artifact --verbose
```

Where `$config` can be:
  - `RelWithDebInfo` (you probably want this one)
  - `Debug` for debugging (very verbose outputs) 
