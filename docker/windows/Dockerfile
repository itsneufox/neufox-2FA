# escape=`

FROM mcr.microsoft.com/windows/servercore:ltsc2022

# Build arguments
ARG ARCH=x64

# Install Chocolatey
RUN powershell -Command `
    Set-ExecutionPolicy Bypass -Scope Process -Force; `
    [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; `
    iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))

# Install build tools
RUN choco install -y `
    git `
    cmake --installargs 'ADD_CMAKE_TO_PATH=System' `
    python3 `
    visualstudio2022buildtools --package-parameters "--add Microsoft.VisualStudio.Workload.VCTools --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 --add Microsoft.VisualStudio.Component.Windows11SDK.22621 --includeRecommended --includeOptional --passive --norestart"

# Install vcpkg
RUN powershell -Command `
    cd C:\; `
    git clone https://github.com/Microsoft/vcpkg.git; `
    cd vcpkg; `
    .\bootstrap-vcpkg.bat

# Set environment variables
ENV VCPKG_ROOT=C:\vcpkg
ENV PATH="${PATH};C:\vcpkg"

# Create build user and workspace
RUN mkdir C:\build
WORKDIR C:\build

# Copy build script
COPY build-entrypoint.ps1 C:\
ENTRYPOINT ["powershell", "-File", "C:\\build-entrypoint.ps1"]

