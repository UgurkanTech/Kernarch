#ifndef FAT32_H
#define FAT32_H

#include "disk.h"
#include "types.h"

#define FAT_ENTRY_SIZE 4
#define DIR_ENTRY_SIZE 32
#define CLUSTER_SIZE 4096  // 4KB clusters, adjust as needed

struct FAT32BootSector {
    uint8_t  jump_boot[3];
    uint8_t  oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t  num_fats;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t  media;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint32_t fat_size_32;
    uint16_t ext_flags;
    uint16_t fs_version;
    uint32_t root_cluster;
    uint16_t fs_info;
    uint16_t backup_boot_sector;
    uint8_t  reserved[12];
    uint8_t  drive_number;
    uint8_t  reserved1;
    uint8_t  boot_signature;
    uint32_t volume_id;
    uint8_t  volume_label[11];
    uint8_t  fs_type[8];
} __attribute__((packed));

struct FAT32DirEntry {
    uint8_t  name[11];
    uint8_t  attributes;
    uint8_t  nt_reserved;
    uint8_t  creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t first_cluster_high;
    uint16_t last_write_time;
    uint16_t last_write_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
} __attribute__((packed));

class FAT32 {
public:
    static FAT32* instance();
    
    bool initialize(Disk* disk);
    bool format_disk(uint32_t total_sectors, const char* volume_label = "NO NAME    ");
    bool create_file(const char* filename);
    bool delete_file(const char* filename);
    bool read_file(const char* filename, void* buffer, size_t* size);
    bool write_file(const char* filename, const void* buffer, size_t size);
    bool list_directory(const char* path);

private:
    FAT32();
    FAT32(const FAT32&) = delete;
    FAT32& operator=(const FAT32&) = delete;

    static FAT32* s_instance;
    Disk* m_disk;
    FAT32BootSector m_boot_sector;
    uint32_t m_fat_start_sector;
    uint32_t m_data_start_sector;
    uint32_t m_sectors_per_cluster;

    bool write_boot_sector(uint32_t total_sectors, const char* volume_label);
    void calculate_fat_values(uint32_t total_sectors);
    bool read_boot_sector();
    uint32_t cluster_to_sector(uint32_t cluster);
    uint32_t get_next_cluster(uint32_t current_cluster);
    bool read_cluster(uint32_t cluster, void* buffer);
    bool write_cluster(uint32_t cluster, const void* buffer);
    uint32_t find_free_cluster();
    bool set_cluster_value(uint32_t cluster, uint32_t value);
    FAT32DirEntry* find_file_in_directory(uint32_t dir_cluster, const char* filename);
    uint32_t create_file_entry(uint32_t dir_cluster, const char* filename, uint32_t first_cluster, uint32_t file_size);
};

#endif // FAT32_H