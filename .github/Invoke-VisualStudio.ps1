Function Invoke-CmdScript {
    param(
        [String] $scriptName
    )
    $cmdLine = """$scriptName"" $args & set"
    & $env:SystemRoot\system32\cmd.exe /c $cmdLine |
    Select-String '^([^=]*)=(.*)$' | ForEach-Object {
        $varName = $_.Matches[0].Groups[1].Value
        $varValue = $_.Matches[0].Groups[2].Value
        Set-Item Env:$varName $varValue
    }
}


Function Invoke-VisualStudio2019x86 {
    Invoke-CmdScript "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Auxiliary/Build/vcvars32.bat"
}


Function Invoke-VisualStudio2019x64 {
    Invoke-CmdScript "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Auxiliary/Build/vcvars64.bat"
}

Function Invoke-VisualStudio2019arm64 {
    Invoke-CmdScript "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Auxiliary/Build/vcvarsamd64_arm64.bat"
}

Function Invoke-VisualStudio2022x86 {
    Invoke-CmdScript "C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Auxiliary/Build/vcvars32.bat"
}


Function Invoke-VisualStudio2022x64 {
    Invoke-CmdScript "C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Auxiliary/Build/vcvars64.bat"
}

Function Invoke-VisualStudio2022arm64 {
    Invoke-CmdScript "C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Auxiliary/Build/vcvarsamd64_arm64.bat"
}
