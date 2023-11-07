
# This CMake script will build and run a CPUID utility.
# It detects processor features and writes a cpuinfo.json file,
# containing cpu vendor, brand, isa-features and architecture level.
#
# The variable CPUINFO_OK is set in case of a successful compilation and run.
# If successful, we can read the json file, check each feature
# and set CMAKE variables accordingly, e.g. HAS_SSE42, HAS_AVX2, HAS_AVX512.

include (CheckCXXSourceRuns)
include (CMakePushCheckState)

cmake_push_check_state ()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_REQUIRED_FLAGS "-std=c++11 -lstdc++")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_REQUIRED_FLAGS "-std=c++11 -lstdc++")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # /EHsc catches C++ exceptions only and tells the compiler to assume that
  # extern C functions never throw a C++ exception.
  set(CMAKE_REQUIRED_FLAGS "/EHsc /W4")
endif()

# resetting this var is needed to debug CPUINFO_SOURCE_FILE
#unset(CPUINFO_OK CACHE)

set(CPUINFO_SOURCE_FILE
"#include <iostream>
#include <vector>
#include <bitset>
#include <array>
#include <string>
#include <fstream>
#include <sstream>
#ifdef WIN32
#include <intrin.h>
#else
#include <cpuid.h>
#include <string.h>
#endif

class InstructionSet
{
    // forward declarations
    class InstructionSet_Internal;

public:
    // getters
    static std::string Vendor(void) { return CPU_Rep.vendor_; }
    static std::string Brand(void) { return CPU_Rep.brand_; }

    static bool SSE3(void) { return CPU_Rep.f_1_ECX_[0]; }
    static bool PCLMULQDQ(void) { return CPU_Rep.f_1_ECX_[1]; }
    static bool MONITOR(void) { return CPU_Rep.f_1_ECX_[3]; }
    static bool SSSE3(void) { return CPU_Rep.f_1_ECX_[9]; }
    static bool FMA(void) { return CPU_Rep.f_1_ECX_[12]; }
    static bool CMPXCHG16B(void) { return CPU_Rep.f_1_ECX_[13]; }
    static bool SSE41(void) { return CPU_Rep.f_1_ECX_[19]; }
    static bool SSE42(void) { return CPU_Rep.f_1_ECX_[20]; }
    static bool MOVBE(void) { return CPU_Rep.f_1_ECX_[22]; }
    static bool POPCNT(void) { return CPU_Rep.f_1_ECX_[23]; }
    static bool AES(void) { return CPU_Rep.f_1_ECX_[25]; }
    static bool XSAVE(void) { return CPU_Rep.f_1_ECX_[26]; }
    static bool OSXSAVE(void) { return CPU_Rep.f_1_ECX_[27]; }
    static bool AVX(void) { return CPU_Rep.f_1_ECX_[28]; }
    static bool F16C(void) { return CPU_Rep.f_1_ECX_[29]; }
    static bool RDRAND(void) { return CPU_Rep.f_1_ECX_[30]; }

    static bool FPU(void) { return CPU_Rep.f_1_EDX_[0]; }
    static bool MSR(void) { return CPU_Rep.f_1_EDX_[5]; }
    static bool CX8(void) { return CPU_Rep.f_1_EDX_[8]; }
    static bool SEP(void) { return CPU_Rep.f_1_EDX_[11]; }
    static bool CMOV(void) { return CPU_Rep.f_1_EDX_[15]; }
    static bool CLFSH(void) { return CPU_Rep.f_1_EDX_[19]; }
    static bool MMX(void) { return CPU_Rep.f_1_EDX_[23]; }
    static bool FXSR(void) { return CPU_Rep.f_1_EDX_[24]; }
    static bool SSE(void) { return CPU_Rep.f_1_EDX_[25]; }
    static bool SSE2(void) { return CPU_Rep.f_1_EDX_[26]; }

    static bool FSGSBASE(void) { return CPU_Rep.f_7_EBX_[0]; }
    static bool BMI1(void) { return CPU_Rep.f_7_EBX_[3]; }
    static bool HLE(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_7_EBX_[4]; }
    static bool AVX2(void) { return CPU_Rep.f_7_EBX_[5]; }
    static bool BMI2(void) { return CPU_Rep.f_7_EBX_[8]; }
    static bool ERMS(void) { return CPU_Rep.f_7_EBX_[9]; }
    static bool INVPCID(void) { return CPU_Rep.f_7_EBX_[10]; }
    static bool RTM(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_7_EBX_[11]; }
    static bool AVX512F(void) { return CPU_Rep.f_7_EBX_[16]; }
    static bool AVX512DQ(void) { return CPU_Rep.f_7_EBX_[17]; }
    static bool RDSEED(void) { return CPU_Rep.f_7_EBX_[18]; }
    static bool ADX(void) { return CPU_Rep.f_7_EBX_[19]; }
    static bool AVX512PF(void) { return CPU_Rep.f_7_EBX_[26]; }
    static bool AVX512ER(void) { return CPU_Rep.f_7_EBX_[27]; }
    static bool AVX512CD(void) { return CPU_Rep.f_7_EBX_[28]; }
    static bool SHA(void) { return CPU_Rep.f_7_EBX_[29]; }
    static bool AVX512BW(void) { return CPU_Rep.f_7_EBX_[30]; }
    static bool AVX512VL(void) { return CPU_Rep.f_7_EBX_[31]; }

    static bool PREFETCHWT1(void) { return CPU_Rep.f_7_ECX_[0]; }

    static bool LAHF(void) { return CPU_Rep.f_81_ECX_[0]; }
    static bool LZCNT_ABM(void) { return CPU_Rep.f_81_ECX_[5]; }
    static bool SSE4a(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[6]; }
    static bool XOP(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[11]; }
    static bool TBM(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[21]; }

    static bool SYSCALL(void) { return CPU_Rep.f_81_EDX_[11] || (CPU_Rep.isAMD_ && CPU_Rep.f_81_EDX_[10]); }
    static bool MMXEXT(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_EDX_[22]; }
    static bool RDTSCP(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_81_EDX_[27]; }
    static bool _3DNOWEXT(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_EDX_[30]; }
    static bool _3DNOW(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_EDX_[31]; }

    // Check for operating system support through CR4[9] for FXSR and SSE.
    static bool OSFXSR(void) {
        // XXX Should actually check CR4.
        return FXSR() && SSE()
    }

    // Check for operating system support through EFER[0] for syscall instruction.
    static bool SCE(void) {
        // XXX Should actually check EFER.
        return SYSCALL();
    }

    static bool x86_64_v1(void) {
        return CMOV() && CX8() && FPU() && FXSR() && MMX() && OSFXSR() && SCE() && SSE() && SSE2();
    }

    static bool x86_64_v2(void) {
        return x86_64_v1() && CMPXCHG16B() && LAHF() && POPCNT() && SSE3() && SSE41() && SSE42() && SSSE3();
    }

    static bool x86_64_v3(void) {
        return x86_64_v2() && AVX() && AVX2() && BMI1() && BMI2() && FMA() && LZCNT_ABM() && MOVBE() && OSXSAVE();
    }

    static bool x86_64_v4(void) {
        return x86_64_v3() && AVX512F() && AVX512BW() && AVX512CD() && AVX512DQ() && AVX512VL();
    }

private:
    static const InstructionSet_Internal CPU_Rep;

    class InstructionSet_Internal
    {
    public:
        InstructionSet_Internal()
            : nIds_{ 0 },
            nExIds_{ 0 },
            isIntel_{ false },
            isAMD_{ false },
            f_1_ECX_{ 0 },
            f_1_EDX_{ 0 },
            f_7_EBX_{ 0 },
            f_7_ECX_{ 0 },
            f_81_ECX_{ 0 },
            f_81_EDX_{ 0 },
            data_{},
            extdata_{}
        {
#ifdef WIN32
            std::array<int, 4> cpui;
#else
            std::array<unsigned int, 4> cpui;
#endif

            // Calling __cpuid with 0x0 as the function_id argument
            // gets the number of the highest valid function ID.
#ifdef WIN32
            __cpuid(cpui.data(), 0);
#else
            cpui[0] = __get_cpuid_max(0, &cpui[1]);
#endif
            nIds_ = cpui[0];

            for (int i = 0; i <= nIds_; ++i)
            {
#ifdef WIN32
                __cpuidex(cpui.data(), i, 0);
#else
                __cpuid_count(i, 0, cpui[0], cpui[1], cpui[2], cpui[3]);
#endif

                data_.push_back(cpui);
            }

            // Capture vendor string
            char vendor[0x20];
            memset(vendor, 0, sizeof(vendor));
            *reinterpret_cast<int*>(vendor + 0) = data_[0][1];
            *reinterpret_cast<int*>(vendor + 4) = data_[0][3];
            *reinterpret_cast<int*>(vendor + 8) = data_[0][2];
            vendor_ = vendor;
            if (vendor_ == \"GenuineIntel\")
            {
                isIntel_ = true;
            }
            else if (vendor_ == \"AuthenticAMD\")
            {
                isAMD_ = true;
            }

            // load bitset with flags for function 0x00000001
            if (nIds_ >= 1)
            {
                f_1_ECX_ = data_[1][2];
                f_1_EDX_ = data_[1][3];
            }

            // load bitset with flags for function 0x00000007
            if (nIds_ >= 7)
            {
                f_7_EBX_ = data_[7][1];
                f_7_ECX_ = data_[7][2];
            }

            // Calling __cpuid with 0x80000000 as the function_id argument
            // gets the number of the highest valid extended ID.
#ifdef WIN32
            __cpuid(cpui.data(), 0x80000000);
#else
            __cpuid(0x80000000, cpui[0], cpui[1], cpui[2], cpui[3]);
#endif
            nExIds_ = cpui[0];

            char brand[0x40];
            memset(brand, 0, sizeof(brand));

            for (int i = 0x80000000; i <= nExIds_; ++i)
            {
#ifdef WIN32
                __cpuidex(cpui.data(), i, 0);
#else
                __cpuid_count(i, 0, cpui[0], cpui[1], cpui[2], cpui[3]);
#endif
                extdata_.push_back(cpui);
            }

            // load bitset with flags for function 0x80000001
            if (nExIds_ >= 0x80000001)
            {
                f_81_ECX_ = extdata_[1][2];
                f_81_EDX_ = extdata_[1][3];
            }

            // Interpret CPU brand string if reported
            if (nExIds_ >= 0x80000004)
            {
                memcpy(brand +  0, extdata_[2].data(), sizeof(cpui));
                memcpy(brand + 16, extdata_[3].data(), sizeof(cpui));
                memcpy(brand + 32, extdata_[4].data(), sizeof(cpui));
                brand_ = brand;
            }
        };

        int nIds_;
        int nExIds_;
        std::string vendor_;
        std::string brand_;
        bool isIntel_;
        bool isAMD_;
        std::bitset<32> f_1_ECX_;
        std::bitset<32> f_1_EDX_;
        std::bitset<32> f_7_EBX_;
        std::bitset<32> f_7_ECX_;
        std::bitset<32> f_81_ECX_;
        std::bitset<32> f_81_EDX_;
#ifdef WIN32
        std::vector<std::array<int, 4>> data_;
        std::vector<std::array<int, 4>> extdata_;
#else
        std::vector<std::array<unsigned int, 4>> data_;
        std::vector<std::array<unsigned int, 4>> extdata_;
#endif
    };
};

// Initialize static member data
const InstructionSet::InstructionSet_Internal InstructionSet::CPU_Rep;

inline std::string trim(std::string& str)
{
    str.erase(str.find_last_not_of(' ')+1);   // right-trim
    str.erase(0, str.find_first_not_of(' ')); // left-trim
    return str;
}
inline std::string rm_last_char(std::string& str, const std::string& c)
{
    return str.substr(0, str.find_last_of(c));
}

int main()
{
    std::ostringstream outstream;

    // print the json key value pair
    auto print_pair = [&outstream](std::string key, auto val) {
        outstream << \"    \\\\\"\" << key << \"\\\\\": \" << std::boolalpha << val << \",\\\\n\";
    };

    print_pair(\"3DNOW\",       InstructionSet::_3DNOW());
    print_pair(\"3DNOWEXT\",    InstructionSet::_3DNOWEXT());
    print_pair(\"ADX\",         InstructionSet::ADX());
    print_pair(\"AES\",         InstructionSet::AES());
    print_pair(\"AVX\",         InstructionSet::AVX());
    print_pair(\"AVX2\",        InstructionSet::AVX2());
    print_pair(\"AVX512BW\",    InstructionSet::AVX512BW());
    print_pair(\"AVX512CD\",    InstructionSet::AVX512CD());
    print_pair(\"AVX512DQ\",    InstructionSet::AVX512DQ());
    print_pair(\"AVX512ER\",    InstructionSet::AVX512ER());
    print_pair(\"AVX512F\",     InstructionSet::AVX512F());
    print_pair(\"AVX512PF\",    InstructionSet::AVX512PF());
    print_pair(\"AVX512VL\",    InstructionSet::AVX512VL());
    print_pair(\"BMI1\",        InstructionSet::BMI1());
    print_pair(\"BMI2\",        InstructionSet::BMI2());
    print_pair(\"CLFSH\",       InstructionSet::CLFSH());
    print_pair(\"CMPXCHG16B\",  InstructionSet::CMPXCHG16B());
    print_pair(\"CX8\",         InstructionSet::CX8());
    print_pair(\"ERMS\",        InstructionSet::ERMS());
    print_pair(\"F16C\",        InstructionSet::F16C());
    print_pair(\"FMA\",         InstructionSet::FMA());
    print_pair(\"FPU\",         InstructionSet::FPU());
    print_pair(\"FSGSBASE\",    InstructionSet::FSGSBASE());
    print_pair(\"FXSR\",        InstructionSet::FXSR());
    print_pair(\"HLE\",         InstructionSet::HLE());
    print_pair(\"INVPCID\",     InstructionSet::INVPCID());
    print_pair(\"LAHF\",        InstructionSet::LAHF());
    print_pair(\"LZCNT\",       InstructionSet::LZCNT());
    print_pair(\"MMX\",         InstructionSet::MMX());
    print_pair(\"MMXEXT\",      InstructionSet::MMXEXT());
    print_pair(\"MONITOR\",     InstructionSet::MONITOR());
    print_pair(\"MOVBE\",       InstructionSet::MOVBE());
    print_pair(\"MSR\",         InstructionSet::MSR());
    print_pair(\"OSXSAVE\",     InstructionSet::OSXSAVE());
    print_pair(\"PCLMULQDQ\",   InstructionSet::PCLMULQDQ());
    print_pair(\"POPCNT\",      InstructionSet::POPCNT());
    print_pair(\"PREFETCHWT1\", InstructionSet::PREFETCHWT1());
    print_pair(\"RDRAND\",      InstructionSet::RDRAND());
    print_pair(\"RDSEED\",      InstructionSet::RDSEED());
    print_pair(\"RDTSCP\",      InstructionSet::RDTSCP());
    print_pair(\"RTM\",         InstructionSet::RTM());
    print_pair(\"SEP\",         InstructionSet::SEP());
    print_pair(\"SHA\",         InstructionSet::SHA());
    print_pair(\"SSE\",         InstructionSet::SSE());
    print_pair(\"SSE2\",        InstructionSet::SSE2());
    print_pair(\"SSE3\",        InstructionSet::SSE3());
    print_pair(\"SSE4.1\",      InstructionSet::SSE41());
    print_pair(\"SSE4.2\",      InstructionSet::SSE42());
    print_pair(\"SSE4a\",       InstructionSet::SSE4a());
    print_pair(\"SSSE3\",       InstructionSet::SSSE3());
    print_pair(\"SYSCALL\",     InstructionSet::SYSCALL());
    print_pair(\"TBM\",         InstructionSet::TBM());
    print_pair(\"XOP\",         InstructionSet::XOP());
    print_pair(\"XSAVE\",       InstructionSet::XSAVE());

    // remove trailing comma from last item in isa-features
    std::string isa_feature = outstream.str();
    isa_feature = rm_last_char(isa_feature, \",\");

    // determine architecture level
    std::string architecture;
    if(InstructionSet::x86_64_v4()) { architecture = \"x86-64-v4\"; } else
    if(InstructionSet::x86_64_v3()) { architecture = \"x86-64-v3\"; } else
    if(InstructionSet::x86_64_v2()) { architecture = \"x86-64-v2\"; } else
    if(InstructionSet::x86_64_v1()) { architecture = \"x86-64-v1\"; } else
                                    { architecture = \"x86\"; }

    std::string vendor = InstructionSet::Vendor();
    std::string brand = InstructionSet::Brand();
    brand = trim(brand);

    std::string NL = \"\\\\n\";  // double-escaped new line

    // build json document
    // This uses double escape qouting insanity.
    // We embed json into cpp by escaping it, then we embed escaped cpp into cmake.
    std::string json_str =
        \"{\"                                                               +NL+
        \" \\\\\"cpu\\\\\": {\"                                             +NL+
        \"    \\\\\"vendor\\\\\": \\\\\"\" + vendor + \"\\\\\",\"           +NL+
        \"    \\\\\"brand\\\\\": \\\\\"\" + brand + \"\\\\\"\"              +NL+
        \" },\"                                                             +NL+
        \" \\\\\"isa-features\\\\\": {\" + NL + isa_feature + NL + \"  },\" +NL+
        \" \\\\\"architecture\\\\\": \\\\\"\" + architecture + \"\\\\\"\"   +NL+
        \"}\";

    // print to console
    std::cout << json_str;

    // write file
    std::ofstream file(\"${CMAKE_BINARY_DIR}/cpuinfo.json\");
    file << json_str;
    file.close();

    return 0;
}
")

check_cxx_source_runs("${CPUINFO_SOURCE_FILE}" CPUINFO_OK)

cmake_pop_check_state ()

# fail early. this is for debugging the cpp
if(NOT CPUINFO_OK)
    message(FATAL_ERROR "Failed to compile cpuid.cpp source. CMake Exit.")
endif()

if(CPUINFO_OK)
  file(READ "${CMAKE_BINARY_DIR}/cpuinfo.json" CPUINFO_JSON_STRING)

  string(JSON CPUINFO_CPU_OBJECT   GET ${CPUINFO_JSON_STRING} "cpu")
  string(JSON CPUINFO_VENDOR       GET ${CPUINFO_CPU_OBJECT}  "vendor") # cpu.vendor
  string(JSON CPUINFO_BRAND        GET ${CPUINFO_CPU_OBJECT}  "brand")  # cpu.brand

  # example on how to access the isa-features
  #string(JSON CPUINFO_ISA_OBJECT   GET ${CPUINFO_JSON_STRING}   "isa-features")
  #string(JSON HAS_SSE2                GET ${CPUINFO_ISA_OBJECT} "SSE2") # isa-features.SSE2

  string(JSON CPUINFO_ARCHITECTURE_LEVEL   GET ${CPUINFO_JSON_STRING} "architecture")

  message(STATUS "[CPU_INFO] Overview:")
  message(STATUS "[CPU_INFO]  - Dataset            -> ${CMAKE_BINARY_DIR}/cpuinfo.json")
  message(STATUS "[CPU_INFO]  - Vendor             -> ${CPUINFO_VENDOR}")
  message(STATUS "[CPU_INFO]  - Brand              -> ${CPUINFO_BRAND}")
  message(STATUS "[CPU_INFO]  - Architecture Level -> ${CPUINFO_ARCHITECTURE_LEVEL}")

  # architecture levels
  set(HOST_IS_X86_64_1 FALSE)
  set(HOST_IS_X86_64_2 FALSE)
  set(HOST_IS_X86_64_3 FALSE)
  set(HOST_IS_X86_64_4 FALSE)

  if(${CPUINFO_ARCHITECTURE_LEVEL} STREQUAL "x86-64-v4")
    set(HOST_IS_X86_64_4 TRUE)
    set(HOST_IS_X86_64_3 TRUE)
    set(HOST_IS_X86_64_2 TRUE)
    set(HOST_IS_X86_64_1 TRUE)
  elseif(${CPUINFO_ARCHITECTURE_LEVEL} STREQUAL "x86-64-v3")
    set(HOST_IS_X86_64_3 TRUE)
    set(HOST_IS_X86_64_2 TRUE)
    set(HOST_IS_X86_64_1 TRUE)
  elseif(${CPUINFO_ARCHITECTURE_LEVEL} STREQUAL "x86-64-v2")
    set(HOST_IS_X86_64_2 TRUE)
    set(HOST_IS_X86_64_1 TRUE)
  elseif(${CPUINFO_ARCHITECTURE_LEVEL} STREQUAL "x86-64-v1")
    set(HOST_IS_X86_64_1 TRUE)
  else()
    message(WARNING "Architecture level does not match any expected value: ${CPUINFO_ARCHITECTURE_LEVEL}")
  endif()

endif()
