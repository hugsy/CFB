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


## TODO

 - [ ] Use Event (instead of Sleep) for handling new pushed messages
 - [ ] Fix dangling threads that make Fuzzer hangs after exit
 - [ ] Fix C# exception when exiting without driver hooked
 

-- hugsy
