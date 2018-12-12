# Canadian Fuzzy Bear

## Concept

_TODO add pretty diagram here_

## Show Me!

_TODO add pretty screenshot here_


## Build

### GUI

Clone the repository, and build the `CFB.sln` at the project root with Visual Studio (Debug - very verbose - or Release).



### Command line

Clone the repository and in a VS prompt run

```
> msbuild CFB.sln /p:Configuration=Release
```

## Setup

A Windows 7+ VM (I'd recommend a [Windows 10 SDK VM](https://developer.microsoft.com/en-us/windows/downloads/virtual-machines))

On this VM:
 - Enable kernel debug
 - Enable test signing

Install VS 2017 redist x86 or x64 depending on your VM architecture.


-- hugsy
