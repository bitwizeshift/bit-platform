#include <bit/platform/system/processor.hpp>

#include "cpuid.hpp"

#include <cstring>
#include <thread>

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------

bit::platform::processor::processor()
  noexcept
{

  cpuid_info info;

  //--------------------------------------------------------------------------

  execute_cpuid( &info, 0 );

  // Get CPU vendor string
  // auto max_cpuid_standard = info.EAX;

  std::memcpy( &m_vendor_string[0], &info.EBX, 4 );
  std::memcpy( &m_vendor_string[4], &info.EDX, 4 );
  std::memcpy( &m_vendor_string[8], &info.ECX, 4 );

  m_vendor_string[12] = '\0';

  // Check if we are on Intel or AMD (if either)
  if(stl::string_view(m_vendor_string) == "GenuineIntel"){
    m_is_intel = true;
  }else if(stl::string_view(m_vendor_string) == "AuthenticAMD"){
    m_is_amd = true;
  }

  //-------------------------------------------------------------------------

  // Get CPU max extended
  execute_cpuid( &info,0x80000000 );

  auto max_cpuid_extended = info.EAX;

  //-------------------------------------------------------------------------

  // Get CPU Features
  execute_cpuid( &info, 1 );

  stl::u32 cpu_features_ecx = info.ECX;
  stl::u32 cpu_features_edx = info.EDX;

  // EDX Information

  m_has_RDTSC = (cpu_features_edx & (1 << 4));
  m_has_CMOV  = (cpu_features_edx & (1 << 15));
  m_has_FCMOV = (cpu_features_edx & (1 << 15));
  m_has_MMX   = (cpu_features_edx & (1 << 23));
  m_has_SSE   = (cpu_features_edx & (1 << 25));
  m_has_SSE2  = (cpu_features_edx & (1 << 26));
  m_has_HT    = (cpu_features_edx & (1 << 28));

  // ECX Information

  m_has_SSE3   = (cpu_features_ecx & (1));
  m_has_SSSE3  = (cpu_features_ecx & (1 << 9));
  m_has_SSE4_1 = (cpu_features_ecx & (1 << 19));
  m_has_SSE4_2 = (cpu_features_ecx & (1 << 20));
  m_has_AVX    = (cpu_features_ecx & (1 << 28));
  m_has_f16c   = (cpu_features_ecx & (1 << 29));
  m_has_rdrand = (cpu_features_ecx & (1 << 30));

  //-------------------------------------------------------------------------

  m_logical_cores  = std::thread::hardware_concurrency();


  int apic_id_size = 0;
  bool multiprocessor_support = false;
  int nc  = 0;
  int mnc = 0;

  m_has_3DNow = false;

  // 0x80000008 retrieves virtual and physical address spacing
  if(max_cpuid_extended >= 0x80000008)
  {
    execute_cpuid( &info, 0x80000008 );
    apic_id_size = (info.ECX >> 12) & 0xf;
    nc           = (info.ECX) & 0xff;

    if(apic_id_size){
      mnc = (1 << apic_id_size);
    }else{
      mnc = nc + 1;
    }

    m_processors = mnc;
  }
  // 0x80000004 retrieves extended brand information
  if(max_cpuid_extended >= 0x80000004)
  {
    execute_cpuid( &info, 0x80000002 );
    std::memcpy( &m_brand_string[0],  &info.EAX, 4 );
    std::memcpy( &m_brand_string[4],  &info.EBX, 4 );
    std::memcpy( &m_brand_string[8],  &info.ECX, 4 );
    std::memcpy( &m_brand_string[12], &info.EDX, 4 );

    execute_cpuid( &info, 0x80000003 );
    std::memcpy( &m_brand_string[16], &info.EAX, 4 );
    std::memcpy( &m_brand_string[20], &info.EBX, 4 );
    std::memcpy( &m_brand_string[24], &info.ECX, 4 );
    std::memcpy( &m_brand_string[28], &info.EDX, 4 );

    execute_cpuid( &info, 0x80000004 );
    std::memcpy( &m_brand_string[32], &info.EAX, 4 );
    std::memcpy( &m_brand_string[36], &info.EBX, 4 );
    std::memcpy( &m_brand_string[40], &info.ECX, 4 );
    std::memcpy( &m_brand_string[44], &info.EDX, 4 );
    m_brand_string[48] = '\0';
  }
  // 0x80000001 retrieves extended processor features
  if(max_cpuid_extended >= 0x80000001)
  {
    execute_cpuid( &info, 0x80000001 );
    m_has_3DNow = (info.EDX & (1 << 31));
    multiprocessor_support = (info.EDX & (1 << 19));
    if( !multiprocessor_support ){
      m_processors = 1;
    }
  }

}
