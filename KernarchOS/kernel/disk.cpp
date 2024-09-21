#include "disk.h"
#include "logger.h"
#include "fat32.h"
#include "pit.h"
#include "io.h"
#include "cstring.h"

Disk* Disk::s_instance = nullptr;

Disk* Disk::instance() {
    if (!s_instance) {
        s_instance = new Disk();
    }
    return s_instance;
}

Disk::Disk() {
    // Initialize the disk
    ACPI* acpi = ACPI::instance();
    acpi->scan_drives();
    m_drive_info = acpi->get_drive_info(0);  // Assuming we're using the first drive
}

bool Disk::initialize() {
    Logger::info("Initializing disk");
    ACPI* acpi = ACPI::instance();
    m_drive_info = acpi->get_drive_info(0);  // Get the first drive

    if (!m_drive_info.present) {
        Logger::error("No disk drive found");
        return false;
    }

    if (!initialize_drive()) {
        Logger::error("Failed to initialize drive");
        return false;
    }

    return true;
}

bool Disk::initialize_drive() {
    Logger::info("Initializing drive");
    
    // Attempt to reset the drive once
    if (!reset_drive()) {
        Logger::error("Failed to reset drive");
        return false;
    }
    
    // Attempt to identify the drive once
    if (!identify_drive()) {
        Logger::error("Failed to identify drive");
        return false;
    }
    
    Logger::info("Drive initialized successfully");
    return true;
}

bool Disk::read_sectors(uint32_t lba, uint8_t sector_count, void* buffer) const {
    ACPI* acpi = ACPI::instance();
    
    char log_message[100];
    format_string(log_message, sizeof(log_message), "Reading sectors from LBA %u", lba);
    Logger::info(log_message);

    if (!is_ready()) {
        Logger::error("Disk not ready for read operation");
        return false;
    }

    bool success = acpi->read_multiple_sectors(m_drive_info, lba, sector_count, static_cast<uint8_t*>(buffer));
    
    if (!success) {
        format_string(log_message, sizeof(log_message), "Failed to read sectors from LBA %u", lba);
        Logger::error(log_message);
        
        // Log the status of the drive after failed read
        uint8_t status = acpi->drive_read_port(m_drive_info, 7);
        format_string(log_message, sizeof(log_message), "Drive status after failed read: 0x%02X", status);
        Logger::info(log_message);
    } else {
        Logger::info("Read operation completed successfully");
    }

    return success;
}

bool Disk::write_sectors(uint32_t lba, uint8_t sector_count, const void* buffer) {
    return ACPI::instance()->write_multiple_sectors(m_drive_info, lba, sector_count, static_cast<const uint8_t*>(buffer));
}

bool Disk::read_sector(uint32_t lba, void* buffer) {
    return read_sectors(lba, 1, buffer);
}

bool Disk::write_sector(uint32_t lba, const void* buffer) {
    return write_sectors(lba, 1, buffer);
}

uint32_t Disk::get_size() const {
    return ACPI::instance()->get_drive_size(m_drive_info);
}

bool Disk::is_ready() const {
    ACPI* acpi = ACPI::instance();
    uint8_t status = acpi->drive_read_port(m_drive_info, 7);
    return !(status & 0x80) && (status & 0x40);  // BSY clear and DRDY set
}

bool Disk::create_file(const char* filename) {
    return FAT32::instance()->create_file(filename);
}

bool Disk::delete_file(const char* filename) {
    return FAT32::instance()->delete_file(filename);
}

bool Disk::read_file(const char* filename, void* buffer, size_t* size) {
    return FAT32::instance()->read_file(filename, buffer, size);
}

bool Disk::write_file(const char* filename, const void* buffer, size_t size) {
    return FAT32::instance()->write_file(filename, buffer, size);
}

bool Disk::format_as_fat32(const char* volume_label) {
    if (!is_ready()) {
        Logger::error("Disk not ready for formatting");
        return false;
    }

    uint32_t total_sectors = get_size() / 512;  // Assuming 512 bytes per sector

    FAT32* fat32 = FAT32::instance();
    if (!fat32->initialize(this)) {
        Logger::error("Failed to initialize FAT32 for formatting");
        return false;
    }

    if (!fat32->format_disk(total_sectors, volume_label)) {
        Logger::error("FAT32 formatting failed");
        return false;
    }

    Logger::info("Disk successfully formatted as FAT32");
    return true;
}
bool Disk::reset_drive() {
    Logger::info("Resetting drive");
    ACPI* acpi = ACPI::instance();
    
    acpi->drive_write_port(m_drive_info, 7, 0x04);
    pit_sleep(50);  // Wait for 5ms
    
    Logger::info("Waiting for drive to be ready after reset");
    uint32_t timeout = 500;  // 1 second timeout
    while (timeout > 0) {
        uint8_t status = acpi->drive_read_port(m_drive_info, 7);
        if (!(status & 0x80) && (status & 0x40)) {
            Logger::info("Drive reset successful");
            return true;
        }
        pit_sleep(1);
        timeout--;
    }
    
    Logger::info("Drive reset successful");
    return true;
}
bool Disk::is_formatted() const {
    Logger::info("Checking if disk is formatted");
    
    uint8_t boot_sector[512];
    memset(boot_sector, 0, sizeof(boot_sector));  // Initialize buffer to zeros

    // Log drive information
    char log_message[100];

    // Check if drive is ready
    if (!is_ready()) {
        Logger::error("Drive not ready for reading boot sector");
        return false;
    }

    // Attempt to read the boot sector
    Logger::info("Attempting to read boot sector");
    if (!read_sectors(0, 1, boot_sector)) {
        Logger::error("Failed to read boot sector");
        
        // Log the status of the drive after failed read
        uint8_t status = ACPI::instance()->drive_read_port(m_drive_info, 7);
        format_string(log_message, sizeof(log_message), "Drive status after failed read: 0x%02X", status);
        Logger::info(log_message);
        
        return false;
    }

    // Check for FAT32 signature
    bool has_signature = (boot_sector[510] == 0x55 && boot_sector[511] == 0xAA);
    
    if (has_signature) {
        Logger::info("FAT32 signature found");
    } else {
        Logger::info("FAT32 signature not found");
    }

    return has_signature;
}

bool Disk::identify_drive() {
    Logger::info("Identifying drive");
    ACPI* acpi = ACPI::instance();
    
    // Select drive
    acpi->drive_write_port(m_drive_info, 6, 0xA0 | (m_drive_info.is_master ? 0 : 0x10));
    pit_sleep(1);
    
    // Send IDENTIFY command
    acpi->drive_write_port(m_drive_info, 7, 0xEC);
    
    // Wait for BSY to clear
    uint32_t timeout = 1000;  // 1 second timeout
    while (timeout > 0) {
        uint8_t status = acpi->drive_read_port(m_drive_info, 7);
        if (!(status & 0x80)) break;
        pit_sleep(1);
        timeout--;
    }
    
    if (timeout == 0) {
        Logger::error("Drive identification timed out");
        return false;
    }
    
    // Check if drive supports IDENTIFY
    uint8_t status = acpi->drive_read_port(m_drive_info, 7);
    if (status == 0 || status == 0xFF) {
        Logger::error("Drive does not support IDENTIFY command");
        return false;
    }
    
    // Wait for DRQ to set
    timeout = 1000;  // 1 second timeout
    while (timeout > 0) {
        status = acpi->drive_read_port(m_drive_info, 7);
        if (status & 0x08) break;
        pit_sleep(1);
        timeout--;
    }
    
    if (timeout == 0) {
        Logger::error("Drive did not set DRQ after IDENTIFY command");
        return false;
    }
    
    // Read identify data
    uint16_t identify_data[256];
    for (int i = 0; i < 256; i++) {
        identify_data[i] = acpi->drive_read_port(m_drive_info, 0);
    }
    
    // Process identify data (you can add more processing here if needed)
    m_drive_info.size_in_sectors = ((uint32_t)identify_data[61] << 16) | identify_data[60];
    
    Logger::info("Drive identified successfully");
    return true;
}