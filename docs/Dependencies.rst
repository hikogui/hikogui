# Dependencies for TTauri

## Windows configuration
For code analysis with Visual Studio you want to add the following system environment variable, (I did
not find a way to include this environment variable in the CMakeSettings.json):
```
CAExcludePath=C:\VulkanSDK;C:\Program Files;C:\Users\Tjienta\Projects\TTauri\ThirdPartyLibraries
```


## Vulkan SDK
Download at: https://www.lunarg.com/vulkan-sdk/

### Windows install
Install in default location as given by the installer executable.

## Python 3.x
Download at: https://python.org/

### Windows install
Make sure to install it for all users and add Python to the path.

## Doxygen

## Sphynx
### Windows
Sphinx must be installed as Administrator otherwise the sphinx-build executable
is not installed.

pip install sphinx
pip install sphinx_rtd_theme
pip install breathe
