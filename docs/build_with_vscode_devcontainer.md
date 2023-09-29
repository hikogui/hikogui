Building with Visual Studio Code devcontainer
================================================

The Visual Studio Code DevContainers extension lets you use a Docker container
as a full-featured development environment.

Hikogui provides a devcontainer environment for developing on Linux.
The devcontainer is Debian-based and includes GCC, Clang, and the Vulkan SDK.
During the container's setup, our recommended VSCode extensions are installed.

This enables you to simply switch to a Linux container and start developing.

You find the configuration file at `.devcontainer/devcontainer.json`.

## Prerequisites

To run this setup, you'll need the following:

- Docker
  - https://docs.docker.com/desktop/install/windows-install/
  - https://docs.docker.com/desktop/install/linux-install/
- Visual Studio Code
  - https://code.visualstudio.com/download

## How to run this?

1. Get the Source Code:

    Clone this repository using Git or download it as a ZIP archive:

    ```bash
    git clone git@github.com:hikogui/hikogui.git
    ```

2. In Visual Studio Code:

    Open the folder you just obtained, and then reopen the folder within a
    container using the `Dev Containers: Reopen in Container` command.

    This action will initiate the building of the container image, which might
    take some time.

    You can monitor the progress of the container image build in the status bar
    of Visual Studio Code. Look for the message: `Starting Dev Container (show
    log): Building image...`.
    You can also click on it to view the progress in the log output.


3. Wait for the Container Image to Build:

    Be patient while the container image is being built. This process can take
    a while, depending on your system's performance and internet connection.

    The container's size is approximately 4GB unzipped, with 2GB allocated by
    Debian, GCC, Clang, and various tools, and another 2GB allocated by the
    Vulkan SDK.

4. Enjoy! 😎

    Once the container image is successfully built, you can start your
    development work within this environment. Feel free to submit pull requests
    to enhance Linux compatibility. Your contributions are greatly appreciated.
