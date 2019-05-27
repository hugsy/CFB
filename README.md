# Canadian Fuzzy Bear

## Status
Project|Build Status
---|---
GUI|[![Build Status](https://dev.azure.com/blahcat/CFB/_apis/build/status/hugsy.CFB?branchName=master)](https://dev.azure.com/blahcat/CFB/_build/latest?definitionId=1&branchName=master)


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

Install VS 2015/2017/2019 redist x86 or x64 depending on your VM architecture.


-- hugsy
