Hardware Support
================

Processors
----------
Each build of ttauri has different processor requirements.

### x86-64-v2.5
The minimum CPU requirement:
 - **Intel**: Sandy Bridge
 - **AMD**: Jaguar / Bulldozer

Here is a table of microarchitecture levels and the included instruction extensions.

 | Level           | Instruction extensions                               |
 |:-----------     |:---------------------------------------------------- |
 | x86-64          | CMOV CX8 FPU FXSR MMX OSFXSR SCE SSE SSE2            |
 | x86-64-v2       | CMPXCHG16B LAHF-SAHF POPCNT SSE3 SSE4.1 SSE4.2 SSSE3 |
 | **x86-64-v2.5** | **AVX CLMUL AES-NI**                                 |
 | x86-64-v3       | AVX2 BMI1 BMI2 F16C FMA LZCNT MOVBE                  |
 | x86-64-v4       | AVX512F AVX512BW AVX512CD AVX512DQ AVX512VL          |

Here is a table of first CPU from Intel and AMD that supports these microarchitecture levels.

 | Level           | Intel            | AMD                  |
 |:--------------- |:---------------- |:-------------------- |
 | x86-64          | Core             | K8                   |
 | x86-64-v2       | Nehalem          | Jaguar/Bulldozer     |
 | **x86-64-v2.5** | **Sandy Bridge** | **Jaguar/Bulldozer** |
 | x86-64-v3       | Haswell          | Zen                  |
 | x86-64-v4       | Skylake-SP,X     |                      |

This is a table for microarchitecture for each compiler.

 | Level           | MSVC          | gcc                                       |
 |:--------------- |:------------- |:----------------------------------------- |
 | x86-64          |               | -march=x86-64                             |
 | x86-64-v2       |               | -march=x86-64-v2                          |
 | **x86-64-v2.5** | **/arch:AVX** | **-march=x86-64-v2 -mavx -mpclmul -maes** |
 | x86-64-v3       | /arch:AVX2    | -march=x86-64-v3                          |
 | x86-64-v4       | /arch:AVX512  | -march=x86-64-v4                          |


Graphic cards
-------------
TTauri uses vulkan-1.1 API as a minimum.

The oldest graphics cards supporting vulkan-1.1:
 - **NVidia**: GeForce 600 (Kepler)
 - **AMD**: HD 7000 (TeraScale 2)
 - **Intel**: Skylake


