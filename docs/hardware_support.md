Hardware Support
================

Processors
----------
Each build of ttauri has different processor requirements.

### x64-\*-\*
 - **Intel**: 6th generation "Skylake" (2015)
 - **AMD**: 3rd generation "Excavator" (2015)

The following instruction sets are shared between the Skylake and Excavator architectures:

 | Skylake | Excavator |
 |:------- |:--------- |
 | SSE     | SSE       |
 | SSE2    | SSE2      |
 | SSE3    | SSE3      |
 | SSSE3   | SSSE3     |
 | SSE4    | SSE4      |
 | SSE4.1  | SSE4.1    |
 | SSE4.2  | SSE4.2    |
 | ADX     |           |
 | AVX     | AVX       |
 | AVX2    | AVX2      |
 | TSX     |           |
 | FMA3    | FMA3      |
 |         | FMA3      |
 | AES-NI  | AES-NI    |
 | CLMUL   | CLMUL     |
 | RDRAND  | RDRAND    |
 | F16C    | F16C      |
 | BMI2    | BMI2      |

Compiler flags:
 - **MSVC**: /arch:AVX2




Non-discontinues products
-------------------------

### Processors
The oldest non-discontinued processors are:
 - **Intel**: 6th generation "Skylake"
 - **AMD**: Piledriver

### Vulkan
The initial release of Vulkan was on February 16, 2016.

TTauri uses vulkan-1.1 API as a minimum.

The oldest graphics cards supporting vulkan-1.1:
 - **NVidia**: Kepler Architecture (from 2012)
 - **AMD**: E1-2000 (from 2013)
 - **Intel**: 6th Gen "Skylake" processors (from 2015)

### Windows 10
Supports very old processors, including 32 bit processors.

