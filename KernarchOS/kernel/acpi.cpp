#include "acpi.h"
#include "io.h"
#include "terminal.h"
#include "cstring.h"
#include "memory.h" // For kmalloc and kfree

using namespace std;

ACPI::ACPI() : rsdt(nullptr), pm1a_control_block(0), slp_typa(0), slp_typb(0), rsdp(nullptr) {}

ACPI::~ACPI() {
    if (rsdt) {
        kfree(rsdt);
    }
}

bool ACPI::initialize() {
    rsdp = find_rsdp();
    if (!rsdp) {
        term_print("ACPI: RSDP not found\n");
        return false;
    }

    uint32_t rsdt_address = *reinterpret_cast<uint32_t*>(static_cast<char*>(rsdp) + 16);
    ACPISDTHeader* rsdt_header = reinterpret_cast<ACPISDTHeader*>(rsdt_address);
    
    rsdt = static_cast<ACPISDTHeader*>(kmalloc(rsdt_header->Length));
    if (!rsdt) {
        term_print("ACPI: Failed to allocate memory for RSDT\n");
        return false;
    }
    memcpy(rsdt, rsdt_header, rsdt_header->Length);

    parse_fadt();

    return true;
}

void* ACPI::find_rsdp() const {
    const char* addr = reinterpret_cast<const char*>(0x000E0000);
    const char* end = reinterpret_cast<const char*>(0x00100000);
    
    while (addr < end) {
        if (memcmp(addr, "RSD PTR ", 8) == 0) {
            return const_cast<char*>(addr);
        }
        addr += 16;
    }
    return nullptr;
}

ACPISDTHeader* ACPI::find_table(const char* signature) const {
    if (!rsdt) return nullptr;

    uint32_t entries = (rsdt->Length - sizeof(ACPISDTHeader)) / sizeof(uint32_t);
    uint32_t* table_ptrs = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(rsdt) + sizeof(ACPISDTHeader));
    
    for (uint32_t i = 0; i < entries; i++) {
        ACPISDTHeader* h = reinterpret_cast<ACPISDTHeader*>(table_ptrs[i]);
        if (memcmp(h->Signature, signature, 4) == 0) {
            return h;
        }
    }
    return nullptr;
}

void ACPI::parse_fadt() {
    ACPISDTHeader* fadt = find_table("FACP");
    if (fadt) {
        uint8_t* data = reinterpret_cast<uint8_t*>(fadt);
        pm1a_control_block = *reinterpret_cast<uint32_t*>(data + 64);
        slp_typa = *reinterpret_cast<uint16_t*>(data + 116);
        slp_typb = *reinterpret_cast<uint16_t*>(data + 118);
    }
}

ACPI::CPUInfo ACPI::get_cpu_info() const {
    CPUInfo info = {0, 0.0};
    ACPISDTHeader* madt = find_table("APIC");
    if (madt) {
        uint8_t* entry = reinterpret_cast<uint8_t*>(madt) + sizeof(ACPISDTHeader);
        uint8_t* end = reinterpret_cast<uint8_t*>(madt) + madt->Length;

        while (entry < end) {
            uint8_t entry_type = *entry;
            uint8_t entry_length = *(entry + 1);

            if (entry_type == 0) {  // Processor Local APIC
                info.core_count++;
            }

            if (entry_length < 2 || entry + entry_length > end) {
                break;
            }

            entry += entry_length;
        }
    }

    // Note: Getting accurate clock speed requires more complex ACPI methods
    info.clock_speed = 0.0;  // Placeholder

    return info;
}
inline uint64_t max(uint64_t a, uint64_t b) {
    return (a > b) ? a : b;
}


ACPI::SystemInfo ACPI::get_system_info() const {
    SystemInfo info = {{0}, {0}, 0};
    
    if (rsdp) {
        memcpy(info.oem_id, static_cast<char*>(rsdp) + 9, 6);
        info.oem_id[6] = '\0';
        info.acpi_revision = *reinterpret_cast<uint8_t*>(static_cast<char*>(rsdp) + 15);
    }

    ACPISDTHeader* fadt = find_table("FACP");
    if (fadt) {
        memcpy(info.motherboard_name, fadt->OEMTableID, 8);
        info.motherboard_name[8] = '\0';
    }

    return info;
}

ACPI::PowerInfo ACPI::get_power_info() const {
    PowerInfo info = {0.0, 0.0, false, 0};

    // Note: Getting accurate voltage and temperature requires 
    // implementing and calling ACPI methods, which is complex
    info.cpu_voltage = 0.0;  // Placeholder
    info.cpu_temperature = 0.0;  // Placeholder

    // Check for battery (simplified)
    ACPISDTHeader* bat = find_table("BATC");
    info.battery_present = (bat != nullptr);
    info.battery_percentage = 0;  // Placeholder

    return info;
}

void ACPI::handle_power_button() const {
    if (pm1a_control_block) {
        uint16_t PM1_CNT = pm1a_control_block;
        uint16_t SLP_EN = 1 << 13;
        outw(PM1_CNT, slp_typa | SLP_EN);
        if (slp_typb) {
            outw(PM1_CNT + 4, slp_typb | SLP_EN);
        }
    }
}

void ACPI::print_system_info() const {
    CPUInfo cpu_info = get_cpu_info();
    SystemInfo sys_info = get_system_info();
    PowerInfo power_info = get_power_info();

    term_print("ACPI System Information:\n");
    term_print("------------------------\n");
    term_print("CPU Cores: ");
    term_print_uint(cpu_info.core_count);
    term_print("\n");

    term_print("Motherboard: ");
    term_print(sys_info.motherboard_name);
    term_print("\n");

    term_print("OEM ID: ");
    term_print(sys_info.oem_id);
    term_print("\n");

    term_print("ACPI Revision: ");
    term_print_uint(sys_info.acpi_revision);
    term_print("\n");

    term_print("Battery present: ");
    term_print(power_info.battery_present ? "Yes" : "No");
    term_print("\n");
}