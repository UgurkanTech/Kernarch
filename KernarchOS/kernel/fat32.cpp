#include "fat32.h"
#include "logger.h"
#include "cstring.h"

FAT32* FAT32::s_instance = nullptr;

FAT32* FAT32::instance() {
    if (!s_instance) {
        s_instance = new FAT32();
    }
    return s_instance;
}

FAT32::FAT32() : m_disk(nullptr) {}

bool FAT32::initialize(Disk* disk) {
    m_disk = disk;
    if (!read_boot_sector()) {
        Logger::error("Failed to read FAT32 boot sector");
        return false;
    }
    m_fat_start_sector = m_boot_sector.reserved_sector_count;
    m_data_start_sector = m_fat_start_sector + (m_boot_sector.num_fats * m_boot_sector.fat_size_32);
    m_sectors_per_cluster = m_boot_sector.sectors_per_cluster;
    Logger::info("FAT32 initialized");
    return true;
}

bool FAT32::read_boot_sector() {
    return m_disk->read_sectors(0, 1, &m_boot_sector);
}

uint32_t FAT32::cluster_to_sector(uint32_t cluster) {
    return m_data_start_sector + (cluster - 2) * m_sectors_per_cluster;
}

bool FAT32::format_disk(uint32_t total_sectors, const char* volume_label) {
    if (!m_disk) {
        Logger::error("FAT32: Disk not initialized");
        return false;
    }

    // Calculate FAT and data region sizes
    calculate_fat_values(total_sectors);

    // Write boot sector
    if (!write_boot_sector(total_sectors, volume_label)) {
        Logger::error("FAT32: Failed to write boot sector");
        return false;
    }

    // Clear FATs
    uint8_t zero_buffer[512] = {0};
    for (uint32_t i = m_fat_start_sector; i < m_data_start_sector; i++) {
        if (!m_disk->write_sectors(i, 1, zero_buffer)) {
            Logger::error("FAT32: Failed to clear FAT");
            return false;
        }
    }

    // Mark reserved clusters in FAT
    uint32_t fat_sector = m_fat_start_sector;
    uint32_t fat_offset = 0;
    uint32_t cluster_value = 0x0FFFFFF8;  // End of chain marker
    for (int i = 0; i < 2; i++) {
        if (!m_disk->read_sectors(fat_sector, 1, zero_buffer)) {
            Logger::error("FAT32: Failed to read FAT sector");
            return false;
        }
        memcpy(&zero_buffer[fat_offset], &cluster_value, sizeof(cluster_value));
        if (!m_disk->write_sectors(fat_sector, 1, zero_buffer)) {
            Logger::error("FAT32: Failed to write FAT sector");
            return false;
        }
        cluster_value = 0x0FFFFFFF;  // Reserved cluster
    }

    Logger::info("FAT32: Disk formatted successfully");
    return true;
}

bool FAT32::write_boot_sector(uint32_t total_sectors, const char* volume_label) {
    memset(&m_boot_sector, 0, sizeof(m_boot_sector));

    m_boot_sector.jump_boot[0] = 0xEB;
    m_boot_sector.jump_boot[1] = 0x58;
    m_boot_sector.jump_boot[2] = 0x90;
    memcpy(m_boot_sector.oem_name, "MSWIN4.1", 8);
    m_boot_sector.bytes_per_sector = 512;
    m_boot_sector.sectors_per_cluster = m_sectors_per_cluster;
    m_boot_sector.reserved_sector_count = 32;
    m_boot_sector.num_fats = 2;
    m_boot_sector.media = 0xF8;
    m_boot_sector.sectors_per_track = 63;
    m_boot_sector.number_of_heads = 255;
    m_boot_sector.hidden_sectors = 0;
    m_boot_sector.total_sectors_32 = total_sectors;
    m_boot_sector.fat_size_32 = (m_data_start_sector - m_fat_start_sector) / 2;
    m_boot_sector.ext_flags = 0;
    m_boot_sector.fs_version = 0;
    m_boot_sector.root_cluster = 2;
    m_boot_sector.fs_info = 1;
    m_boot_sector.backup_boot_sector = 6;
    m_boot_sector.drive_number = 0x80;
    m_boot_sector.boot_signature = 0x29;
    m_boot_sector.volume_id = 0x12345678;  // You might want to generate this randomly
    memcpy(m_boot_sector.volume_label, volume_label, 11);
    memcpy(m_boot_sector.fs_type, "FAT32   ", 8);

    return m_disk->write_sectors(0, 1, &m_boot_sector);
}

void FAT32::calculate_fat_values(uint32_t total_sectors) {
    uint32_t root_dir_sectors = 0;  // For FAT32, this is always 0
    uint32_t tmp_val1 = total_sectors - (m_boot_sector.reserved_sector_count + root_dir_sectors);
    uint32_t tmp_val2 = (256 * m_sectors_per_cluster) + 2;
    m_boot_sector.fat_size_32 = (tmp_val1 + (tmp_val2 - 1)) / tmp_val2;

    m_fat_start_sector = m_boot_sector.reserved_sector_count;
    m_data_start_sector = m_fat_start_sector + (m_boot_sector.num_fats * m_boot_sector.fat_size_32);
}

uint32_t FAT32::get_next_cluster(uint32_t current_cluster) {
    uint32_t fat_offset = current_cluster * FAT_ENTRY_SIZE;
    uint32_t fat_sector = m_fat_start_sector + (fat_offset / m_boot_sector.bytes_per_sector);
    uint32_t entry_offset = fat_offset % m_boot_sector.bytes_per_sector;
    
    uint8_t sector_buffer[512];
    if (!m_disk->read_sectors(fat_sector, 1, sector_buffer)) {
        return 0xFFFFFFFF;  // Invalid cluster
    }
    
    return *(uint32_t*)(&sector_buffer[entry_offset]) & 0x0FFFFFFF;
}

bool FAT32::read_cluster(uint32_t cluster, void* buffer) {
    uint32_t start_sector = cluster_to_sector(cluster);
    return m_disk->read_sectors(start_sector, m_sectors_per_cluster, buffer);
}

bool FAT32::write_cluster(uint32_t cluster, const void* buffer) {
    uint32_t start_sector = cluster_to_sector(cluster);
    return m_disk->write_sectors(start_sector, m_sectors_per_cluster, buffer);
}

bool FAT32::create_file(const char* filename) {
    uint32_t free_cluster = find_free_cluster();
    if (free_cluster == 0) {
        Logger::error("No free clusters available");
        return false;
    }

    uint32_t root_dir_cluster = m_boot_sector.root_cluster;
    if (!create_file_entry(root_dir_cluster, filename, free_cluster, 0)) {
        Logger::error("Failed to create file entry");
        return false;
    }

    if (!set_cluster_value(free_cluster, 0x0FFFFFFF)) {
        Logger::error("Failed to mark end of file");
        return false;
    }

    Logger::info("File created successfully");
    return true;
}


uint32_t FAT32::find_free_cluster() {
    uint8_t fat_buffer[512];
    for (uint32_t fat_sector = 0; fat_sector < m_boot_sector.fat_size_32; fat_sector++) {
        if (!m_disk->read_sectors(m_fat_start_sector + fat_sector, 1, fat_buffer)) {
            continue;
        }
        for (uint32_t i = 0; i < 512; i += FAT_ENTRY_SIZE) {
            uint32_t cluster_value = *(uint32_t*)(&fat_buffer[i]) & 0x0FFFFFFF;
            if (cluster_value == 0) {
                return (fat_sector * 128) + (i / FAT_ENTRY_SIZE);
            }
        }
    }
    return 0;  // No free clusters found
}

bool FAT32::set_cluster_value(uint32_t cluster, uint32_t value) {
    uint32_t fat_offset = cluster * FAT_ENTRY_SIZE;
    uint32_t fat_sector = m_fat_start_sector + (fat_offset / m_boot_sector.bytes_per_sector);
    uint32_t entry_offset = fat_offset % m_boot_sector.bytes_per_sector;
    
    uint8_t sector_buffer[512];
    if (!m_disk->read_sectors(fat_sector, 1, sector_buffer)) {
        return false;
    }
    
    *(uint32_t*)(&sector_buffer[entry_offset]) = value & 0x0FFFFFFF;
    
    return m_disk->write_sectors(fat_sector, 1, sector_buffer);
}

FAT32DirEntry* FAT32::find_file_in_directory(uint32_t dir_cluster, const char* filename) {
    static uint8_t cluster_buffer[CLUSTER_SIZE];
    static FAT32DirEntry* dir_entry = (FAT32DirEntry*)cluster_buffer;

    while (dir_cluster < 0x0FFFFFF8) {
        if (!read_cluster(dir_cluster, cluster_buffer)) {
            return nullptr;
        }

        for (uint32_t i = 0; i < CLUSTER_SIZE; i += DIR_ENTRY_SIZE) {
            if (dir_entry[i / DIR_ENTRY_SIZE].name[0] == 0) {
                return nullptr;  // End of directory
            }
            if (strncmp((char*)dir_entry[i / DIR_ENTRY_SIZE].name, filename, 11) == 0) {
                return &dir_entry[i / DIR_ENTRY_SIZE];
            }
        }

        dir_cluster = get_next_cluster(dir_cluster);
    }

    return nullptr;
}

uint32_t FAT32::create_file_entry(uint32_t dir_cluster, const char* filename, uint32_t first_cluster, uint32_t file_size) {
    static uint8_t cluster_buffer[CLUSTER_SIZE];
    static FAT32DirEntry* dir_entry = (FAT32DirEntry*)cluster_buffer;

    while (dir_cluster < 0x0FFFFFF8) {
        if (!read_cluster(dir_cluster, cluster_buffer)) {
            return 0;
        }

        for (uint32_t i = 0; i < CLUSTER_SIZE; i += DIR_ENTRY_SIZE) {
            if (dir_entry[i / DIR_ENTRY_SIZE].name[0] == 0 || dir_entry[i / DIR_ENTRY_SIZE].name[0] == 0xE5) {
                // Found a free entry
                memset(&dir_entry[i / DIR_ENTRY_SIZE], 0, DIR_ENTRY_SIZE);
                strncpy((char*)dir_entry[i / DIR_ENTRY_SIZE].name, filename, 11);
                dir_entry[i / DIR_ENTRY_SIZE].attributes = 0x20;  // Archive bit set
                dir_entry[i / DIR_ENTRY_SIZE].first_cluster_low = first_cluster & 0xFFFF;
                dir_entry[i / DIR_ENTRY_SIZE].first_cluster_high = (first_cluster >> 16) & 0xFFFF;
                dir_entry[i / DIR_ENTRY_SIZE].file_size = file_size;

                if (!write_cluster(dir_cluster, cluster_buffer)) {
                    return 0;
                }

                return dir_cluster;
            }
        }

        dir_cluster = get_next_cluster(dir_cluster);
    }

    return 0;  // No free entries found
}

bool FAT32::delete_file(const char* filename) {
    uint32_t root_dir_cluster = m_boot_sector.root_cluster;
    FAT32DirEntry* file_entry = find_file_in_directory(root_dir_cluster, filename);
    
    if (!file_entry) {
        Logger::error("File not found");
        return false;
    }

    file_entry->name[0] = 0xE5;

    uint32_t dir_sector = cluster_to_sector(root_dir_cluster);
    if (!m_disk->write_sectors(dir_sector, 1, file_entry)) {
        Logger::error("Failed to update directory entry");
        return false;
    }

    uint32_t cluster = (file_entry->first_cluster_high << 16) | file_entry->first_cluster_low;
    if (!set_cluster_value(cluster, 0)) {
        Logger::error("Failed to free cluster");
        return false;
    }

    Logger::info("File deleted successfully");
    return true;
}

bool FAT32::read_file(const char* filename, void* buffer, size_t* size) {
    uint32_t root_dir_cluster = m_boot_sector.root_cluster;
    FAT32DirEntry* file_entry = find_file_in_directory(root_dir_cluster, filename);
    
    if (!file_entry) {
        Logger::error("File not found: %s", filename);
        return false;
    }

    uint32_t file_size = file_entry->file_size;
    if (*size < file_size) {
        Logger::error("Buffer too small to read entire file");
        return false;
    }

    *size = file_size;
    uint32_t cluster = (file_entry->first_cluster_high << 16) | file_entry->first_cluster_low;
    uint8_t* buf = static_cast<uint8_t*>(buffer);
    size_t bytes_read = 0;

    while (cluster < 0x0FFFFFF8 && bytes_read < file_size) {
        if (!read_cluster(cluster, buf + bytes_read)) {
            Logger::error("Failed to read cluster");
            return false;
        }
        bytes_read += CLUSTER_SIZE;
        cluster = get_next_cluster(cluster);
    }

    Logger::info("File read: %s (%d bytes)", filename, *size);
    return true;
}

bool FAT32::write_file(const char* filename, const void* buffer, size_t size) {
    uint32_t root_dir_cluster = m_boot_sector.root_cluster;
    FAT32DirEntry* file_entry = find_file_in_directory(root_dir_cluster, filename);
    
    if (file_entry) {
        // File exists, delete it first
        if (!delete_file(filename)) {
            Logger::error("Failed to delete existing file");
            return false;
        }
    }

    // Create a new file
    uint32_t first_cluster = find_free_cluster();
    if (first_cluster == 0) {
        Logger::error("No free clusters available");
        return false;
    }

    if (!create_file_entry(root_dir_cluster, filename, first_cluster, size)) {
        Logger::error("Failed to create file entry");
        return false;
    }

    // Write file contents
    const uint8_t* buf = static_cast<const uint8_t*>(buffer);
    size_t bytes_written = 0;
    uint32_t current_cluster = first_cluster;

    while (bytes_written < size) {
        size_t chunk_size = (size - bytes_written < CLUSTER_SIZE) ? size - bytes_written : CLUSTER_SIZE;
        if (!write_cluster(current_cluster, buf + bytes_written)) {
            Logger::error("Failed to write cluster");
            return false;
        }
        bytes_written += chunk_size;

        if (bytes_written < size) {
            uint32_t next_cluster = find_free_cluster();
            if (next_cluster == 0) {
                Logger::error("No free clusters available");
                return false;
            }
            if (!set_cluster_value(current_cluster, next_cluster)) {
                Logger::error("Failed to link clusters");
                return false;
            }
            current_cluster = next_cluster;
        }
    }

    // Mark the end of the file
    if (!set_cluster_value(current_cluster, 0x0FFFFFFF)) {
        Logger::error("Failed to mark end of file");
        return false;
    }

    Logger::info("File written: %s (%d bytes)", filename, size);
    return true;
}