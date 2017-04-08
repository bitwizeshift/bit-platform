#include "cpuid.hpp"

// If compiling on MSVC for windows
#if defined(BIT_COMPILER_MSVC) && defined(BIT_PLATFORM_WINDOWS)
#  include <intrin.h>
#endif

  // MSVC has custom routines for this exact purpose
#if defined(BIT_COMPILER_MSVC)
#  if defined(BIT_ARCHITECTURE_32)

  // MSVC makes this work for both 64 and 32 bit
bool bit::platform::is_cpuid_supported()
  noexcept
{
  __try{
    // Try assembly to see if it works
    __asm{
      xor edx, edx
      mov eax, 1
      cpuid
    }
  }__except(EXCEPTION_EXECUTE_HANDLER){
    return false;
  }
  return true;
}

#  elif defined(BIT_ARCHITECTURE_64)

  // 64bit MSVC disallows assembly; so assume support
bool bit::platform::is_cpuid_supported()
  noexcept
{
  return true;
}

#  endif

void bit::platform::execute_cpuid( cpuid_info* result, u32 function, u32 subfunction )
{
  __cpuidex( static_cast<unsigned int*>(&result), function, subfunction );
}

#elif (defined(BIT_PROCESSOR_X86) || defined(BIT_PROCESSOR_X86_64))

#  if defined(BIT_COMPILER_ASM_STYLE_ATT)
#    if defined(BIT_ARCHITECTURE_32)

bool bit::platform::is_cpuid_supported()
  noexcept
{
  return true; // todo: Detect processor support for 32-bit x86
}

void bit::platform::execute_cpuid( cpuid_info* result, u32 function, u32 subfunction )
{
  asm volatile(
    "cpuid" :
    "=a" (result->EAX),
    "=b" (result->EBX),
    "=c" (result->ECX),
    "=d" (result->EDX)
    : "a" (function),
      "c" (subfunction)
  );
}

#    elif defined(BIT_ARCHITECTURE_64)

bool bit::platform::is_cpuid_supported()
  noexcept
{
  return true; // Almost all 64-bit processors support CPUID
}

void bit::platform::execute_cpuid( cpuid_info* result,
                                stl::u32 function,
                                stl::u32 subfunction )
{
  asm volatile(
    "cpuid" :
    "=a" (result->EAX),
    "=b" (result->EBX),
    "=c" (result->ECX),
    "=d" (result->EDX)
    : "a" (function),
  "c" (subfunction)
  );
}
#    else
#      define BIT_CPUID_NOT_SUPPORTED 1
#    endif

#  elif defined(BIT_COMPILER_ASM_STYLE_INTEL)
#    if defined(BIT_ARCHITECTURE_32)

bool bit::platform::is_cpuid_supported()
  noexcept
{
  return true; // todo: Somehow detect when true
}

void bit::platform::execute_cpuid( cpuid_info* result, u32 function, u32 subfunction )
{
  __asm {
    mov    rdi, result
    mov    eax, function
    mov    ecx, subfunction
    cpuid
    mov    [rdi+0],  eax
    mov    [rdi+4],  ebx
    mov    [rdi+8],  ecx
    mov    [rdi+12], edx
 }

#    elif defined(BIT_ARCHITECTURE_64)

bool bit::platform::is_cpuid_supported()
  noexcept
{
  return true; // Almost all 64-bit processors support CPUID
}

void bit::platform::execute_cpuid( cpuid_info* result, u32 function, u32 subfunction )
{
  __asm {
    mov    rdi, result
    mov    eax, function
    mov    ecx, subfunction
    cpuid
    mov    [rdi+0],  eax
    mov    [rdi+4],  ebx
    mov    [rdi+8],  ecx
    mov    [rdi+12], edx
 }

#    else
#      define BIT_CPUID_NOT_SUPPORTED 1
#    endif
#  endif
#endif


// If no CPUID support was detected, define an invalid version
#ifdef BIT_CPUID_NOT_SUPPORTED

bool bit::platform::is_cpuid_supported()
  noexcept
{
  return false;
}

void bit::platform::execute_cpuid( cpuid_info*, u32, u32 )
{
  BIT_ASSERT(false, "cpuid not supported or implemented with this instruction set");
}

#endif
