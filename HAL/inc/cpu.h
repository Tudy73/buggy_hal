#pragma once

#include "register.h"

#define NO_OF_TOTAL_INTERRUPTS          256
#define NO_OF_RESERVED_EXCEPTIONS       32

#define NO_OF_USABLE_INTERRUPTS         (NO_OF_TOTAL_INTERRUPTS - NO_OF_RESERVED_EXCEPTIONS)

#pragma pack(push,1)

// warning C4201: nonstandard extension used : nameless struct/union
#pragma warning(disable:4201)

// Control and system registers related information

// CR0 related definitions
#define CR0_PE                                      ((QWORD)1<<0)
#define CR0_ET                                      ((QWORD)1<<4)
#define CR0_NE                                      ((QWORD)1<<5)
#define CR0_WP                                      ((QWORD)1<<16)
#define CR0_NW                                      ((QWORD)1<<29)
#define CR0_CD                                      ((QWORD)1<<30)
#define CR0_PG                                      ((QWORD)1<<31)

// CR4 related definitions
#define CR4_PAE                                     ((QWORD)1<<5)
#define CR4_VMXE                                    ((QWORD)1<<13)
#define CR4_SMXE                                    ((QWORD)1<<14)
#define CR4_PCIDE                                   ((QWORD)1<<17)
#define CR4_OSXSAVE                                 ((QWORD)1<<18)
#define CR4_SMEP                                    ((QWORD)1<<20)
#define CR4_SMAP                                    ((QWORD)1<<21)

// RFLAGS related definitions
#define RFLAGS_CARRY_FLAG_BIT                       ((QWORD)1<<0)
#define RFLAGS_RESERVED_BIT                         ((QWORD)1<<1)
#define RFLAGS_TRAP_BIT                             ((QWORD)1<<8)
#define RFLAGS_INTERRUPT_FLAG_BIT                   ((QWORD)1<<9)
#define RFLAGS_DIRECTION_BIT                        ((QWORD)1<<10)
#define RFLAGS_RESUME_FLAG_BIT                      ((QWORD)1<<16)

typedef enum _CPUID_IDX
{
    CpuidIdxBasicInformation                    = 0x0,
    CpuidIdxFeatureInformation,
    CpuidIdxMonitorLeaf                         = 0x5,
    CpuidIdxStructuredExtendedFeaturesLeaf      = 0x7,
    CpuidIdxArchPerfMonLeaf                     = 0xA,
    CpuidIdxExtendedMaxFunction                 = 0x8000'0000,
    CpuidIdxExtendedFeatureInformation          = 0x8000'0001,
    CpuidIdxProcessorAddressSizes               = 0x8000'0008,
} CPUID_IDX;

// CPUID related information

// 0x0
typedef struct _CPUID_BASIC_INFORMATION
{
    DWORD           MaxValueForBasicInfo;
    DWORD           CpuIdString0;
    DWORD           CpuidString2;
    DWORD           CpuidString1;
} CPUID_BASIC_INFORMATION, *PCPUID_BASIC_INFORMATION;
STATIC_ASSERT(sizeof(CPUID_BASIC_INFORMATION) == sizeof(DWORD) * 4);

// 0x1
typedef struct _CPUID_EBX_FEATURE_INFORMATION
{
    BYTE            BrandIndex;

    // This value * 8 => cache line size in bytes ( used by CLFLUSHOPT too)
    BYTE            CLFlushLineSize;

    BYTE            MaxNumberOfAddressableIds;

    BYTE            ApicId;
} CPUID_EBX_FEATURE_INFORMATION, *PCPUID_EBX_FEATURE_INFORMATION;    
STATIC_ASSERT(sizeof(CPUID_EBX_FEATURE_INFORMATION) == sizeof(DWORD));

typedef struct _CPUID_ECX_FEATURE_INFORMATION
{
    DWORD           SSE3                        :   1;
    DWORD           PCLMULQDQ                   :   1;
    DWORD           DTES64                      :   1;
    DWORD           MONITOR                     :   1;
    DWORD           DS_CPL                      :   1;
    DWORD           VMX                         :   1;
    DWORD           SMX                         :   1;
    DWORD           EIST                        :   1;
    DWORD           TM2                         :   1;
    DWORD           SSSE3                       :   1;
    DWORD           CNXT_ID                     :   1;
    DWORD           SDBG                        :   1;
    DWORD           FMA                         :   1;
    DWORD           CMPXCHG16B                  :   1;
    DWORD           xTPRUpdateControl           :   1;
    DWORD           PDCM                        :   1;
    DWORD           __Reserved0                 :   1;
    DWORD           PCID                        :   1;
    DWORD           DCA                         :   1;
    DWORD           SSE4_1                      :   1;
    DWORD           SSE4_2                      :   1;
    DWORD           x2APIC                      :   1;
    DWORD           MOVBE                       :   1;
    DWORD           POPCNT                      :   1;
    DWORD           TSC_Deadline                :   1;
    DWORD           AESNI                       :   1;
    DWORD           XSAVE                       :   1;
    DWORD           OSXSAVE                     :   1;
    DWORD           AVX                         :   1;
    DWORD           F16C                        :   1;
    DWORD           RDRAND                      :   1;
    DWORD           HV                          :   1;
} CPUID_ECX_FEATURE_INFORMATION, *PCPUID_ECX_FEATURE_INFORMATION;
STATIC_ASSERT(sizeof(CPUID_ECX_FEATURE_INFORMATION) == sizeof(DWORD));

typedef struct _CPUID_EDX_FEATURE_INFORMATION
{
    DWORD           FPU                         :   1;
    DWORD           VME                         :   1;
    DWORD           DE                          :   1;
    DWORD           PSE                         :   1;
    DWORD           TSC                         :   1;
    DWORD           MSR                         :   1;
    DWORD           PAE                         :   1;
    DWORD           MCE                         :   1;
    DWORD           CX8                         :   1;
    DWORD           APIC                        :   1;
    DWORD           __Reserved0                 :   1;
    DWORD           SEP                         :   1;
    DWORD           MTRR                        :   1;
    DWORD           PGE                         :   1;
    DWORD           MCA                         :   1;
    DWORD           CMOV                        :   1;
    DWORD           PAT                         :   1;
    DWORD           PSE_36                      :   1;
    DWORD           PSN                         :   1;
    DWORD           CLFSH                       :   1;
    DWORD           __Reserved1                 :   1;
    DWORD           DS                          :   1;
    DWORD           ACPI                        :   1;
    DWORD           MMX                         :   1;
    DWORD           FXSR                        :   1;
    DWORD           SSE                         :   1;
    DWORD           SSE2                        :   1;
    DWORD           SS                          :   1;
    DWORD           HTT                         :   1;
    DWORD           TM                          :   1;
    DWORD           __Reserved2                 :   1;
    DWORD           PBE                         :   1;
} CPUID_EDX_FEATURE_INFORMATION, *PCPUID_EDX_FEATURE_INFORMATION;
STATIC_ASSERT(sizeof(CPUID_EDX_FEATURE_INFORMATION) == sizeof(DWORD));

typedef struct _CPUID_FEATURE_INFORMATION
{
    DWORD                               eax;
    CPUID_EBX_FEATURE_INFORMATION       ebx;
    CPUID_ECX_FEATURE_INFORMATION       ecx;
    CPUID_EDX_FEATURE_INFORMATION       edx;
} CPUID_FEATURE_INFORMATION, *PCPUID_FEATURE_INFORMATION;
STATIC_ASSERT(sizeof(CPUID_FEATURE_INFORMATION) == sizeof(DWORD) * 4);

// 0x5
typedef struct _CPUID_EAX_MONITOR_LEAF
{
    WORD                                SmallestMonitorLineSize;
    WORD                                __Reserved0;
} CPUID_EAX_MONITOR_LEAF, *PCPUID_EAX_MONITOR_LEAF;
STATIC_ASSERT(sizeof(CPUID_EAX_MONITOR_LEAF) == sizeof(DWORD));

typedef struct _CPUID_EBX_MONITOR_LEAF
{
    WORD                                LargestMonitorLineSize;
    WORD                                __Reserved0;
} CPUID_EBX_MONITOR_LEAF, *PCPUID_EBX_MONITOR_LEAF;
STATIC_ASSERT(sizeof(CPUID_EBX_MONITOR_LEAF) == sizeof(DWORD));

typedef struct _CPUID_MONITOR_LEAF
{
    CPUID_EAX_MONITOR_LEAF              eax;
    CPUID_EBX_MONITOR_LEAF              ebx;
    DWORD                               ecx;
    DWORD                               edx;
} CPUID_MONITOR_LEAF, *PCPUID_MONITOR_LEAF;
STATIC_ASSERT(sizeof(CPUID_MONITOR_LEAF) == sizeof(DWORD) * 4);

// 0x7, 0x0
typedef struct _CPUID_EBX_STRUCTURED_EXTENDED_FEATURE_FLAGS_LEAF
{
    DWORD                               FSGSBASE                                : 1;
    DWORD                               IA32_TSC_ADJUST_MSR                     : 1;
    DWORD                               SGX                                     : 1;
    DWORD                               BMI1                                    : 1;
    DWORD                               HLE                                     : 1;
    DWORD                               AVX2                                    : 1;
    DWORD                               FP_EXCEPTIN_ONLY                        : 1;
    DWORD                               SMEP                                    : 1;
    DWORD                               BMI2                                    : 1;
    DWORD                               EnhancedRepMovsb                        : 1;
    DWORD                               INVPCID                                 : 1;
    DWORD                               RTM                                     : 1;
    DWORD                               RDTM                                    : 1;
    DWORD                               DeprecatesFpuDsCs                       : 1;
    DWORD                               MPX                                     : 1;
    DWORD                               RDTA                                    : 1;
    DWORD                               __Reserved16_17                         : 2;
    DWORD                               RDSEED                                  : 1;
    DWORD                               ADX                                     : 1;
    DWORD                               SMAP                                    : 1;
    DWORD                               __Reserved21_22                         : 2;
    DWORD                               CLFLUSHOPT                              : 1;
    DWORD                               __Reserved24                            : 1;
    DWORD                               ProcessorTrace                          : 1;
    DWORD                               __Reserved26_28                         : 3;
    DWORD                               SHA                                     : 1;
    DWORD                               __Reserved30_31                         : 2;
} CPUID_EBX_STRUCTURED_EXTENDED_FEATURE_FLAGS_LEAF, *PCPUID_EBX_STRUCTURED_EXTENDED_FEATURE_FLAGS_LEAF;
STATIC_ASSERT(sizeof(CPUID_EBX_STRUCTURED_EXTENDED_FEATURE_FLAGS_LEAF) == sizeof(DWORD));

typedef struct _CPUID_ECX_STRUCTURED_EXTENDED_FEATURE_FLAGS_LEAF
{
    DWORD                               PREFETCHWT1                             : 1;
    DWORD                               __Reserved1                             : 1;
    DWORD                               UMIP                                    : 1;
    DWORD                               PKU                                     : 1;
    DWORD                               OSPKE                                   : 1;
    DWORD                               __Reserved5_16                          : 12;
    DWORD                               MAWAU                                   : 5;
    DWORD                               RDPID                                   : 1;
    DWORD                               __Reserved23_29                         : 7;
    DWORD                               SGX_LC                                  : 1;
    DWORD                               __Reserved31                            : 1;
} CPUID_ECX_STRUCTURED_EXTENDED_FEATURE_FLAGS_LEAF, *PCPUID_ECX_STRUCTURED_EXTENDED_FEATURE_FLAGS_LEAF;
STATIC_ASSERT(sizeof(CPUID_ECX_STRUCTURED_EXTENDED_FEATURE_FLAGS_LEAF) == sizeof(DWORD));

typedef struct _CPUID_STRUCTURED_EXTENDED_FEATURE_FLAGS_LEAF
{
    DWORD                                               MaxSubleaf;
    CPUID_EBX_STRUCTURED_EXTENDED_FEATURE_FLAGS_LEAF    ebx;
    CPUID_ECX_STRUCTURED_EXTENDED_FEATURE_FLAGS_LEAF    ecx;
    DWORD                                               __Reserved;
} CPUID_STRUCTURED_EXTENDED_FEATURE_FLAGS_LEAF, *PCPUID_STRUCTURED_EXTENDED_FEATURE_FLAGS_LEAF;
STATIC_ASSERT(sizeof(CPUID_STRUCTURED_EXTENDED_FEATURE_FLAGS_LEAF) == sizeof(DWORD) * 4);

// 0xA
typedef struct _CPUID_EAX_ARCH_PERF_MON_LEAF
{
    BYTE                                VersionId;
    BYTE                                NumberOfPmcsPerCpu;
    BYTE                                BitWidth;
    BYTE                                LengthOfEbxBitVector;
} CPUID_EAX_ARCH_PERF_MON_LEAF, *PCPUID_EAX_ARCH_PERF_MON_LEAF;
STATIC_ASSERT(sizeof(CPUID_EAX_ARCH_PERF_MON_LEAF) == sizeof(DWORD));

typedef struct _CPUID_EBX_ARCH_PERF_MON_LEAF
{
    DWORD                               CoreCycleEventNotAvail                  : 1;
    DWORD                               InstructionRetiredEventNotAvail         : 1;
    DWORD                               ReferenceCyclesEventNotAvail            : 1;
    DWORD                               LastLevelCacheReferenceEventNotAvail    : 1;
    DWORD                               LastLevelCacheMissesEventNotAvail       : 1;
    DWORD                               BranchInstructionRetiredEventNotAvail   : 1;
    DWORD                               BranchMispredictRetiredEventNotAvail    : 1;
    DWORD                               __Reserved7_31                          : 25;
} CPUID_EBX_ARCH_PERF_MON_LEAF, *PCPUID_EBX_ARCH_PERF_MON_LEAF;
STATIC_ASSERT(sizeof(CPUID_EBX_ARCH_PERF_MON_LEAF) == sizeof(DWORD));

typedef struct _CPUID_EDX_ARCH_PERF_MON_LEAF
{
    // If VersionID > 1
    DWORD                               NoOfFixedPmcs                           : 5;

    // If VersionID > 1
    DWORD                               BitWidthOfFixedPmcs                     : 8;

    DWORD                               __Reserved13_31                         : 19;
} CPUID_EDX_ARCH_PERF_MON_LEAF, *PCPUID_EDX_ARCH_PERF_MON_LEAF;
STATIC_ASSERT(sizeof(CPUID_EDX_ARCH_PERF_MON_LEAF) == sizeof(DWORD));

typedef struct _CPUID_ARCH_PERF_MON_LEAF
{
    CPUID_EAX_ARCH_PERF_MON_LEAF        eax;
    CPUID_EBX_ARCH_PERF_MON_LEAF        ebx;
    DWORD                               ecx;
    CPUID_EDX_ARCH_PERF_MON_LEAF        edx;
} CPUID_ARCH_PERF_MON_LEAF, *PCPUID_ARCH_PERF_MON_LEAF;
STATIC_ASSERT(sizeof(CPUID_ARCH_PERF_MON_LEAF) == sizeof(DWORD) * 4);

// 0x8000'0000
typedef struct _CPUID_EXTENDED_CPUID_INFORMATION
{
    DWORD                               MaxValueForExtendedInfo;
    DWORD                               __Reserved0;
    DWORD                               __Reserved1;
    DWORD                               __Reserved2;
} CPUID_EXTENDED_CPUID_INFORMATION, *PCPUID_EXTENDED_CPUID_INFORMATION;
STATIC_ASSERT(sizeof(CPUID_EXTENDED_CPUID_INFORMATION) == sizeof(DWORD) * 4);

// 0x8000'0001
typedef struct _CPUID_ECX_EXTENDED_FEATURE_INFORMATION
{
    DWORD                               LahfSahf            : 1;
    DWORD                               __Reserved0         : 4;
    DWORD                               LZCNT               : 1;
    DWORD                               __Reserved1         : 2;
    DWORD                               PREFETCHW           : 1;
    DWORD                               __Reserved2         : 23;
} CPUID_ECX_EXTENDED_FEATURE_INFORMATION, *PCPUID_ECX_EXTENDED_FEATURE_INFORMATION;
STATIC_ASSERT(sizeof(CPUID_ECX_EXTENDED_FEATURE_INFORMATION) == sizeof(DWORD));

typedef struct _CPUID_EDX_EXTENDED_FEATURE_INFORMATION
{
    DWORD                               __Reserved0         : 11;
    DWORD                               Syscall             : 1;
    DWORD                               __Reserved1         : 8;
    DWORD                               ExecuteDisable      : 1;
    DWORD                               __Reserved2         : 5;
    DWORD                               LargePages          : 1;
    DWORD                               RDTSCP              : 1;
    DWORD                               __Reserved3         : 1;
    DWORD                               Intel64             : 1;
    DWORD                               __Reserved4         : 2;
} CPUID_EDX_EXTENDED_FEATURE_INFORMATION, *PCPUID_EDX_EXTENDED_FEATURE_INFORMATION;
STATIC_ASSERT(sizeof(CPUID_EDX_EXTENDED_FEATURE_INFORMATION) == sizeof(DWORD));

typedef struct _CPUID_EXTENDED_FEATURE_INFORMATION
{
    DWORD                                   ExtendedProcessorSignature;
    DWORD                                   __Reserved;
    CPUID_ECX_EXTENDED_FEATURE_INFORMATION  ecx;
    CPUID_EDX_EXTENDED_FEATURE_INFORMATION  edx;
} CPUID_EXTENDED_FEATURE_INFORMATION, *PCPUID_EXTENDED_FEATURE_INFORMATION;
STATIC_ASSERT(sizeof(CPUID_EXTENDED_FEATURE_INFORMATION) == sizeof(DWORD) * 4);

// 0x8000'0008
typedef struct _CPUID_EAX_PROCESSOR_ADDRESS_SIZES_INFORMATION
{
    BYTE                                    PhysicalAddressBits;
    BYTE                                    LinearAddressBits;
    WORD                                    __Reserved;
} CPUID_EAX_PROCESSOR_ADDRESS_SIZES_INFORMATION, *PCPUID_EAX_PROCESSOR_ADDRESS_SIZES_INFORMATION;
STATIC_ASSERT(sizeof(CPUID_EAX_PROCESSOR_ADDRESS_SIZES_INFORMATION) == sizeof(DWORD));

typedef struct _CPUID_PROCESSOR_ADDRESS_SIZES_INFORMATION
{
    CPUID_EAX_PROCESSOR_ADDRESS_SIZES_INFORMATION   eax;
    DWORD                                           __Reserved0;
    DWORD                                           __Reserved1;
    DWORD                                           __Reserved2;
} CPUID_PROCESSOR_ADDRESS_SIZES_INFORMATION, *PCPUID_PROCESSOR_ADDRESS_SIZES_INFORMATION;
STATIC_ASSERT(sizeof(CPUID_PROCESSOR_ADDRESS_SIZES_INFORMATION) == sizeof(DWORD) * 4);

// structure retrieved by __cpuid operations
typedef struct _CPUID_INFO
{
    union {
        int values[4];
        struct {
            DWORD eax;
            DWORD ebx;
            DWORD ecx;
            DWORD edx;
        };
        // 0x0
        CPUID_BASIC_INFORMATION                     BasicInformation;

        // 0x1
        CPUID_FEATURE_INFORMATION                   FeatureInformation;
        
        // 0x5
        CPUID_MONITOR_LEAF                          MonitorLeaf;

        // 0x7
        CPUID_STRUCTURED_EXTENDED_FEATURE_FLAGS_LEAF    StructuredExtendedFeatures;

        // 0xA
        CPUID_ARCH_PERF_MON_LEAF                    ArchitecturalPerfMonLeaf;
    
        // 0x8000'0000
        CPUID_EXTENDED_CPUID_INFORMATION            ExtendedInformation;

        // 0x8000'0001
        CPUID_EXTENDED_FEATURE_INFORMATION          ExtendedFeatures;

        // 0x8000'0008
        CPUID_PROCESSOR_ADDRESS_SIZES_INFORMATION   CpuAddressSizes;
    };
} CPUID_INFO, *PCPUID_INFO;
STATIC_ASSERT(sizeof(CPUID_INFO) == sizeof(DWORD) * 4);

#pragma warning(default:4201)
#pragma pack(pop)

// define explicit packing for exported structures
#pragma pack(push,8)
typedef struct _PROCESSOR_STATE
{
    QWORD       RegisterValues[RegisterR15 + 1];

    QWORD       Rip;

    QWORD       Rflags;
} PROCESSOR_STATE, *PPROCESSOR_STATE;
#pragma pack(pop)

__forceinline
extern
void
CpuClearDirectionFlag(
    void
    )
{
    __writeeflags(__readeflags() & (~RFLAGS_DIRECTION_BIT));
}

__forceinline
extern
INTR_STATE
CpuIntrGetState(
    void
    )
{
    return IsBooleanFlagOn(__readeflags(), RFLAGS_INTERRUPT_FLAG_BIT);
}

__forceinline
extern
INTR_STATE
CpuIntrSetState(
    const      INTR_STATE         IntrState
    )
{
    QWORD rFlags = __readeflags();
    QWORD newFlags = IntrState ? ( rFlags | RFLAGS_INTERRUPT_FLAG_BIT ) : ( rFlags & ( ~RFLAGS_INTERRUPT_FLAG_BIT));

    __writeeflags(newFlags);

    return IsBooleanFlagOn(rFlags, RFLAGS_INTERRUPT_FLAG_BIT);
}

__forceinline
extern
INTR_STATE
CpuIntrDisable(
    void
    )
{
    return CpuIntrSetState(FALSE);
}

__forceinline
extern
INTR_STATE
CpuIntrEnable(
    void
    )
{
    return CpuIntrSetState(TRUE);
}

typedef BYTE APIC_ID;

__forceinline
extern
APIC_ID
CpuGetApicId(
    void
    )
{
    CPUID_INFO cpuId;

    __cpuid(cpuId.values, CpuidIdxFeatureInformation);

    return cpuId.FeatureInformation.ebx.ApicId;
}

__forceinline
extern
BOOLEAN
CpuIsIntel(
    void
    )
{
    CPUID_INFO cpuId;

    __cpuid(cpuId.values, CpuidIdxBasicInformation);

    return ( cpuId.ebx == 'uneG' &&
             cpuId.edx == 'Ieni' &&
             cpuId.ecx == 'letn' );
}