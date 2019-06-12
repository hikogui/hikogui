# Dependencies for TTauri

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
