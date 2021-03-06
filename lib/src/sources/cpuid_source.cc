#include <internal/sources/cpuid_source.hpp>

using namespace whereami::sources;
using namespace std;

namespace whereami { namespace sources {

    cpuid_registers cpuid_base::read_cpuid(unsigned int leaf, unsigned int subleaf) const {
        cpuid_registers result;
#if defined(__i386__) && defined(__PIC__)
        // ebx is used for PIC purposes on i386, so we need to manually
        // back it up. The compiler's register allocator can't do this
        // for us, because ebx is permanently reserved in its view.
        // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=47602
        // We may not need this at all on modern GCCs, but ¯\_(ツ)_/¯
        asm volatile(
            "mov %%ebx,%k1; cpuid; xchgl %%ebx,%k1"
            : "=a" (result.eax), "=&r" (result.ebx), "=c" (result.ecx), "=d" (result.edx)
            : "a" (leaf), "c" (subleaf));
#elif defined(__i386__) || defined(__x86_64__)
        asm volatile(
            "cpuid"
            : "=a" (result.eax), "=b" (result.ebx), "=c" (result.ecx), "=d" (result.edx)
            : "a" (leaf), "c" (subleaf));
#endif
        return result;
    }

    bool cpuid_base::has_hypervisor() const
    {
        auto regs = read_cpuid(HYPERVISOR_PRESENT);
        return static_cast<bool>(regs.ecx & (1 << 31));
    }

    string cpuid_base::interpret_vendor_registers(cpuid_registers const& regs) const
    {
        unsigned int result[4] = {regs.ebx, regs.ecx, regs.edx, 0};
        return string {reinterpret_cast<char*>(result)};
    }

    string cpuid_base::vendor(unsigned int subleaf) const
    {
        auto regs = read_cpuid(VENDOR_LEAF, subleaf);
        return interpret_vendor_registers(regs);
    }

    bool cpuid_base::has_vendor(string const& vendor_search) const
    {
        auto regs = read_cpuid(VENDOR_LEAF);
        // CPUID returns the maximum queryable leaf value in eax
        unsigned int max_entries = regs.eax;

        // These bounds are the minimum and maximum sane results for Xen offsets
        if (max_entries < 4 || max_entries >= 0x10000) {
            return (interpret_vendor_registers(regs) == vendor_search);
        }

        // Look through valid offsets until the desired vendor is found or we run out
        for (unsigned int leaf = VENDOR_LEAF + 0x100;
             leaf <= VENDOR_LEAF + max_entries;
             leaf += 0x100) {
            if (interpret_vendor_registers(read_cpuid(leaf)) == vendor_search) {
                return true;
            }
        }

        return false;
    }

}}
