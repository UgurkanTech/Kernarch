#ifndef DISK_H
#define DISK_H

#include "acpi.h"
#include "types.h"

class Disk {
public:
    static Disk* instance();
    
    bool initialize();

    bool initialize_drive();
    bool read_sectors(uint32_t lba, uint8_t sector_count, void* buffer) const;
    bool write_sectors(uint32_t lba, uint8_t sector_count, const void* buffer);
    
    // File system related functions
    bool create_file(const char* filename);
    bool delete_file(const char* filename);
    bool read_file(const char* filename, void* buffer, size_t* size);
    bool write_file(const char* filename, const void* buffer, size_t size);
    
    uint32_t get_size() const;
    bool is_ready() const;

    bool is_formatted() const;

    bool format_as_fat32(const char* volume_label = "NO NAME    ");
private:
    Disk();  // Private constructor for singleton
    Disk(const Disk&) = delete;  // Prevent copying
    Disk& operator=(const Disk&) = delete;  // Prevent assignment
    
    static Disk* s_instance;
    ACPI::DriveInfo m_drive_info;

    bool reset_drive();
    bool identify_drive();
    
    bool read_sector(uint32_t lba, void* buffer);
    bool write_sector(uint32_t lba, const void* buffer);
};

#endif // DISK_H