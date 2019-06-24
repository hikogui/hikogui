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

## Boost
Download at: https://www.boost.org/

### Windows install
Unpack .7z file in: `c:\Program Files\boost\boost_1_70_0`

Execute the following in that directory:
```
bootstrap.bat
b2 link=static
```

Set the `BOOST_ROOT` environment variable to `c:\Program Files\boost\boost_1_70_0`

## Flex and Bison
Download at: https://github.com/lexxmark/winflexbison/releases

### Windows install
Unpack the .zip file in: `c:\Program Files\win_flex_bison`

add `c:\Program Files\win_flex_bison` to PATH environment variable.


## Python 3.x
Download at: https://python.org/

### Windows install
Make sure to install it for all users and add Python to the path.
