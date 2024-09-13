#ifndef ACPI_H
#define ACPI_H

#include <stdint.h>

struct ACPISDTHeader {
    char Signature[4];
    uint32_t Length;
    uint8_t Revision;
    uint8_t Checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
};

class ACPI {
public:
    ACPI();
    ~ACPI();
    bool initialize();

    struct CPUInfo {
        int core_count;
        float clock_speed;  // in GHz
    };

    struct SystemInfo {
        char motherboard_name[64];
        char oem_id[7];
        uint32_t acpi_revision;
    };

    struct PowerInfo {
        float cpu_voltage;
        float cpu_temperature;
        bool battery_present;
        int battery_percentage;
    };

    CPUInfo get_cpu_info() const;
    SystemInfo get_system_info() const;
    PowerInfo get_power_info() const;

    void handle_power_button() const;
    void print_system_info() const;

private:
    ACPISDTHeader* find_table(const char* signature) const;
    void* find_rsdp() const;
    void parse_fadt();
    uint64_t detect_memory() const;

    ACPISDTHeader* rsdt;
    uint32_t pm1a_control_block;
    uint16_t slp_typa;
    uint16_t slp_typb;
    void* rsdp;
};

#endif // ACPI_H