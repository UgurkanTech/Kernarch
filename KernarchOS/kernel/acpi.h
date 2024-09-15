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
    static ACPI* instance();
    static bool initialize();
    ~ACPI();

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


    struct DriveInfo {
        bool present;
        bool is_master;
        char model[41];  // 40 characters + null terminator
        unsigned int size_in_sectors;
        uint16_t base_port;  // Base I/O port for this drive
        uint8_t drive_select;  // Value to select this drive (0xA0 for master, 0xB0 for slave)
    };

    void scan_drives();
    DriveInfo get_drive_info(int drive_index) const;
    void print_drive_info() const;

    void handle_primary_ide_interrupt();
    void handle_secondary_ide_interrupt();

    uint16_t drive_read_port(const DriveInfo& drive, int reg) const;
    void drive_write_port(const DriveInfo& drive, int reg, uint16_t value) const;
    void drive_select(const DriveInfo& drive);
    void read_sector(const DriveInfo& drive, uint32_t lba, uint8_t* buffer);

private:
    ACPI();  // Private constructor
    ACPI(const ACPI&) = delete;  // Prevent copying
    ACPI& operator=(const ACPI&) = delete;  // Prevent assignment

    bool init();

    static ACPI* s_instance;
    static bool s_initialized;    
    
    ACPISDTHeader* find_table(const char* signature) const;
    void* find_rsdp() const;
    void parse_fadt();
    uint64_t detect_memory() const;

    ACPISDTHeader* rsdt;
    uint32_t pm1a_control_block;
    uint16_t slp_typa;
    uint16_t slp_typb;
    void* rsdp;

    static const int MAX_DRIVES = 4;  // 2 IDE channels, 2 drives each
    DriveInfo drives[MAX_DRIVES];

    void detect_ide_drive(int channel, bool is_master);
    uint16_t ide_read_port(int channel, int reg);
    void ide_write_port(int channel, int reg, uint16_t value);
    void ide_read_buffer(int channel, int reg, void* buffer, unsigned int quads);

    void delay(int ms);

};

#endif // ACPI_H