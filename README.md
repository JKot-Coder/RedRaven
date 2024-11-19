# OpenDemo
Simpe graphics demo

### How to Build

#### Install VS Code:
1. **Install Extensions:**
    ```sh
    code --install-extension josetr.cmake-language-support-vscode
    code --install-extension ms-vscode.cmake-tools
    code --install-extension ms-vscode.cpptools
    code --install-extension ms-vscode.cpptools-extension-pack
    code --install-extension shardulm94.trailing-spaces
    ```
2. **Optional Extensions:**
    ```sh
    code --install-extension ms-vscode.cpptools-themes
    code --install-extension jbockle.jbockle-format-files
    code --install-extension jeff-hykin.better-cpp-syntax
    ```

#### Install Development Tools:
1. **Build Tools for Visual Studio:**
    - [Build Tools for Visual Studio 2022](https://visualstudio.microsoft.com/downloads/)
2. **Scoop Package Manager:**
    ```sh
    Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
    Invoke-RestMethod -Uri https://get.scoop.sh | Invoke-Expression
    ```
3. **Install Development Tools:**
    ```sh
    scoop install cmake git ninja
    scoop install main/llvm
    scoop install main/vcpkg
    ```
4. **Set LLVM Path Variables:**
    ```sh
    set LLVMInstallDir=%USERPROFILE%\scoop\apps\llvm\current
    set LLVMToolsVersion=XX.X.X  # Replace with actual version based on `scoop info llvm`
    ```
5. **Configure Cmake:**
    ```sh
    cmake -B build --preset debug
    ```