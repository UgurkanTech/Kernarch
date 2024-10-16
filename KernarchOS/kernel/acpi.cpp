#include "acpi.h"
#include "io.h"
#include "terminal.h"
#include "cstring.h"
#include "memory.h"
#include "logger.h"
#include "string_utils.h"
#include "pit.h"

using namespace std;

ACPI* ACPI::s_instance = nullptr;
bool ACPI::s_initialized = false;

ACPI* ACPI::instance() {
    if (!s_initialized) {
        initialize();
    }
    return s_instance;
}

bool ACPI::initialize() {
    if (s_initialized) {
        return true;
    }

    s_instance = new ACPI();
    if (!s_instance) {
        Logger::log(LogLevel::ERROR, "ACPI: Failed to allocate memory for ACPI instance");
        return false;
    }

    s_initialized = s_instance->init();
    if (!s_initialized) {
        delete s_instance;
        s_instance = nullptr;
    }
    
    Logger::log(LogLevel::INFO, "ACPI initialized");

    return s_initialized;
}

ACPI::ACPI() 
    : rsdt(nullptr), rsdp(nullptr), pm1a_control_block(0), slp_typa(0), slp_typb(0) {
    for (int i = 0; i < MAX_DRIVES; i++) {
        drives[i] = DriveInfo{};
    }
}

ACPI::~ACPI() {
    if (rsdt) {
        kfree(rsdt);
    }
    s_instance = nullptr;
    s_initialized = false;
}

bool ACPI::init() {
    rsdp = find_rsdp();
    if (!rsdp) {
        Logger::log(LogLevel::ERROR, "ACPI: RSDP not found");
        return false;
    }

    Logger::log(LogLevel::DEBUG, "RSDP found");

    // Get RSDT address from RSDP
    uint32_t rsdt_address = rsdp->RsdtAddress;
    Logger::log(LogLevel::DEBUG, "RSDT address");


    //rsdt = reinterpret_cast<ACPISDTHeader*>(rsdt_address); //Fix this

    if (!rsdt) {
        Logger::log(LogLevel::ERROR, "ACPI: Failed to map RSDT");
        return false;
    }

    // Validate RSDT
    if (memcmp(rsdt->Signature, "RSDT", 4) != 0) {
        Logger::log(LogLevel::ERROR, "ACPI: Invalid RSDT signature");
        return false;
    }

    // Parse the FADT
    parse_fadt();

    return true;
}

RSDPDescriptor* ACPI::find_rsdp() const {
    const uintptr_t start_addr = 0x000E0000;
    const uintptr_t end_addr = 0x00100000;

    for (uintptr_t addr = start_addr; addr < end_addr; addr += 16) {
        if (memcmp(reinterpret_cast<void*>(addr), "RSD PTR ", 8) == 0) {
            RSDPDescriptor* potential_rsdp = reinterpret_cast<RSDPDescriptor*>(addr);
            
            // Validate checksum
            uint8_t sum = 0;
            for (size_t i = 0; i < sizeof(RSDPDescriptor); i++) {
                sum += reinterpret_cast<uint8_t*>(potential_rsdp)[i];
            }
            
            if (sum == 0) {
                return potential_rsdp; // Valid RSDP found
            }
        }
    }

    Logger::log(LogLevel::ERROR, "ACPI: RSDP not found in the specified memory range");
    return nullptr;
}

ACPISDTHeader* ACPI::find_table(const char* signature) const {
    if (!rsdt) {
        Logger::log(LogLevel::ERROR, "ACPI: RSDT is null in find_table");
        return nullptr;
    }

    uint32_t entries = (rsdt->Length - sizeof(ACPISDTHeader)) / sizeof(uint32_t);
    uint32_t* table_ptrs = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(rsdt) + sizeof(ACPISDTHeader));
    
    for (uint32_t i = 0; i < entries; i++) {
        ACPISDTHeader* h = reinterpret_cast<ACPISDTHeader*>(table_ptrs[i]);
        
        if (!h) {
            Logger::log(LogLevel::ERROR, "ACPI: Invalid table pointer in find_table");
            continue;
        }

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
        memcpy(info.oem_id, reinterpret_cast<char*>(rsdp) + 9, 6);
        info.oem_id[6] = '\0';
        info.acpi_revision = *reinterpret_cast<uint8_t*>(reinterpret_cast<char*>(rsdp) + 15);
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

void ACPI::delay(int ms) {
    for (int i = 0; i < ms * 1000; i++) {
        __asm__ volatile("nop");
    }
}

void ACPI::scan_drives() {
    Logger::log(LogLevel::DEBUG, "Starting IDE drive scan...");
    detect_ide_drive(0, true);
    Logger::log(LogLevel::DEBUG, "Finished scanning primary master");
    delay(100);
    detect_ide_drive(0, false);
    Logger::log(LogLevel::DEBUG, "Finished scanning primary slave");
    delay(100);
    detect_ide_drive(1, true);
    Logger::log(LogLevel::DEBUG, "Finished scanning secondary master");
    delay(100);
    detect_ide_drive(1, false);
    Logger::log(LogLevel::DEBUG, "Finished scanning secondary slave");
    Logger::log(LogLevel::INFO, "IDE drive scan complete");
}


void ACPI::detect_ide_drive(int channel, bool is_master) {
    int drive_index = channel * 2 + (is_master ? 0 : 1);
    
    if (drive_index < 0 || drive_index >= MAX_DRIVES) {
        Logger::log(LogLevel::ERROR, "Invalid drive index");
        return;
    }

    DriveInfo& drive = drives[drive_index];
    drive.present = false;
    drive.is_master = is_master;
    drive.base_port = channel ? 0x170 : 0x1F0;
    drive.drive_select = is_master ? 0xA0 : 0xB0;

    char log_message[100];
    format_string(log_message, sizeof(log_message), "Detecting drive: Channel %d %s", channel, is_master ? "Master" : "Slave");
    Logger::log(LogLevel::DEBUG, log_message);

    // Select drive
    ide_write_port(channel, 6, drive.drive_select);
    delay(10);

    // Simple presence check
    uint16_t status = ide_read_port(channel, 7);
    if (status == 0xFF00) {
        Logger::log(LogLevel::DEBUG, "No drive detected");
        return;
    }

    Logger::log(LogLevel::DEBUG, "Drive might be present. Attempting IDENTIFY command...");

    // Send IDENTIFY command
    ide_write_port(channel, 7, 0xEC);
    delay(10);

    // Wait for BSY to clear and DRQ to set
    int timeout = 100;
    while (timeout > 0) {
        status = ide_read_port(channel, 7);
        if (!(status & 0x80) && (status & 0x08)) break;  // BSY cleared and DRQ set
        if (status & 0x01) {  // ERR set
            Logger::log(LogLevel::DEBUG, "Error occurred during IDENTIFY");
            return;
        }
        delay(1);
        timeout--;
    }

    if (timeout == 0) {
        Logger::log(LogLevel::ERROR, "Drive timed out");
        return;
    }

    // Read identification data
    uint16_t identify_data[256];
    for (int i = 0; i < 256; i++) {
        identify_data[i] = ide_read_port(channel, 0);
    }

    // Basic sanity check
    if (identify_data[0] == 0x0000 || identify_data[0] == 0xFFFF) {
        Logger::log(LogLevel::ERROR, "Invalid identification data");
        return;
    }

    drive.present = true;

    // Extract model name (words 27-46)
    for (int i = 0; i < 20; i++) {
        uint16_t data = identify_data[27 + i];
        drive.model[i*2] = (data >> 8) & 0xFF;
        drive.model[i*2 + 1] = data & 0xFF;
    }
    drive.model[40] = '\0';

    // Extract drive size (words 60-61 for 28-bit LBA)
    drive.size_in_sectors = ((unsigned int)identify_data[61] << 16 | identify_data[60]);

    format_string(log_message, sizeof(log_message), "Drive detected: %s", drive.model);
    Logger::log(LogLevel::DEBUG, log_message);
}

ACPI::DriveInfo ACPI::get_drive_info(int drive_index) const {
    if (drive_index >= 0 && drive_index < MAX_DRIVES) {
        return drives[drive_index];
    }
    return DriveInfo{};  // Return empty struct if invalid index
}

void ACPI::print_drive_info() const {
    term_print("Detected drives:\n");
    for (int i = 0; i < MAX_DRIVES; i++) {
        if (drives[i].present) {
            char info_message[200];
            unsigned int size_in_mb = drives[i].size_in_sectors / 2048; // 2048 = (1024*1024) / 512

            if (size_in_mb >= 1024) {
                unsigned int size_in_gb = size_in_mb / 1024;
                unsigned int size_in_mb_remainder = size_in_mb % 1024;
                format_string(info_message, sizeof(info_message), 
                    "Drive %d: %s\n"
                    "  Size: %u.%03u GB\n"
                    "  Type: %s\n"
                    "  Sectors: %u\n",
                    i, drives[i].model,
                    size_in_gb, size_in_mb_remainder, 
                    drives[i].is_master ? "Master" : "Slave",
                    drives[i].size_in_sectors
                );
            } else {
                format_string(info_message, sizeof(info_message), 
                    "Drive %d: %s\n"
                    "  Size: %u MB\n"
                    "  Type: %s\n"
                    "  Sectors: %u\n",
                    i, drives[i].model,
                    size_in_mb,
                    drives[i].is_master ? "Master" : "Slave",
                    drives[i].size_in_sectors
                );
            }
            term_print(info_message);
        }
    }
}

void ACPI::handle_primary_ide_interrupt() {
    ide_read_port(0, 7);
}

void ACPI::handle_secondary_ide_interrupt() {
    ide_read_port(1, 7);
}

uint16_t ACPI::ide_read_port(int channel, int reg) {
    uint16_t port = (channel ? 0x170 : 0x1F0) + reg;
    return inw(port);
}

void ACPI::ide_write_port(int channel, int reg, uint16_t value) {
    uint16_t port = (channel ? 0x170 : 0x1F0) + reg;
    outw(port, value);
}

void ACPI::ide_read_buffer(int channel, int reg, void* buffer, unsigned int quads) {
    uint16_t base = channel ? 0x170 : 0x1F0;
    uint16_t port = base + reg - 0x200;
    uint16_t* buf = (uint16_t*)buffer;
    
    for (unsigned int i = 0; i < quads * 2; i++) {
        buf[i] = inw(port);
    }
}

void ACPI::drive_write_port(const DriveInfo& drive, int reg, uint8_t value) const {
    outb(drive.base_port + reg, value);
}

uint8_t ACPI::drive_read_port(const DriveInfo& drive, int reg) const {
    return inb(drive.base_port + reg);
}

bool ACPI::wait_for_drive(const DriveInfo& drive, uint8_t mask, uint8_t value, uint32_t timeout_ms) const {
    uint32_t start_time = pit_get_ticks();
    uint8_t status = drive_read_port(drive, 7);
    
    if ((status & mask) == value) {
        Logger::info("Drive ready");
        return true;
    }
    
    char log_message[100];
    format_string(log_message, sizeof(log_message), "Drive not ready. Status: 0x%02X", status);
    Logger::error(log_message);
    return false;
}

void ACPI::drive_select(const DriveInfo& drive) {
    drive_write_port(drive, 6, drive.drive_select);
    delay(10);  // Small delay after selecting the drive
}

void ACPI::read_sector(const DriveInfo& drive, uint32_t lba, uint8_t* buffer) {
    drive_select(drive);
    
    // Send LBA and sector count
    drive_write_port(drive, 2, 1);  // Sector count = 1
    drive_write_port(drive, 3, lba & 0xFF);
    drive_write_port(drive, 4, (lba >> 8) & 0xFF);
    drive_write_port(drive, 5, (lba >> 16) & 0xFF);
    drive_write_port(drive, 6, 0xE0 | ((lba >> 24) & 0x0F) | drive.drive_select);

    // Send read command
    drive_write_port(drive, 7, 0x20);

    // Wait for data
    while (!(drive_read_port(drive, 7) & 0x08));

    // Read data
    for (int i = 0; i < 256; i++) {
        uint16_t data = drive_read_port(drive, 0);
        buffer[i*2] = data & 0xFF;
        buffer[i*2 + 1] = (data >> 8) & 0xFF;
    }
}

bool ACPI::write_sector(const DriveInfo& drive, uint32_t lba, const uint8_t* buffer) {
    drive_select(drive);
    
    // Send LBA and sector count
    drive_write_port(drive, 2, 1);  // Sector count = 1
    drive_write_port(drive, 3, lba & 0xFF);
    drive_write_port(drive, 4, (lba >> 8) & 0xFF);
    drive_write_port(drive, 5, (lba >> 16) & 0xFF);
    drive_write_port(drive, 6, 0xE0 | ((lba >> 24) & 0x0F) | drive.drive_select);

    // Send write command
    drive_write_port(drive, 7, 0x30);

    // Wait for drive to be ready
    if (!wait_for_drive(drive, 0x08, 0x08, 1000)) {
        return false;
    }

    // Write data
    for (int i = 0; i < 256; i++) {
        uint16_t data = buffer[i*2] | (buffer[i*2 + 1] << 8);
        drive_write_port(drive, 0, data);
    }

    // Wait for write to complete
    return wait_for_drive(drive, 0x80, 0x00, 1000);
}

bool ACPI::read_multiple_sectors(const DriveInfo& drive, uint32_t lba, uint8_t sector_count, uint8_t* buffer) {
    drive_select(drive);
    
    // Send LBA and sector count
    drive_write_port(drive, 2, sector_count);
    drive_write_port(drive, 3, lba & 0xFF);
    drive_write_port(drive, 4, (lba >> 8) & 0xFF);
    drive_write_port(drive, 5, (lba >> 16) & 0xFF);
    drive_write_port(drive, 6, 0xE0 | ((lba >> 24) & 0x0F) | drive.drive_select);

    // Send read command
    drive_write_port(drive, 7, 0x20);

    for (int sector = 0; sector < sector_count; sector++) {
        // Wait for data
        if (!wait_for_drive(drive, 0x08, 0x08, 1000)) {
            return false;
        }

        // Read data
        for (int i = 0; i < 256; i++) {
            uint16_t data = drive_read_port(drive, 0);
            buffer[sector*512 + i*2] = data & 0xFF;
            buffer[sector*512 + i*2 + 1] = (data >> 8) & 0xFF;
        }
    }

    return true;
}


bool ACPI::write_multiple_sectors(const DriveInfo& drive, uint32_t lba, uint8_t sector_count, const uint8_t* buffer) {
    drive_select(drive);
    
    // Send LBA and sector count
    drive_write_port(drive, 2, sector_count);
    drive_write_port(drive, 3, lba & 0xFF);
    drive_write_port(drive, 4, (lba >> 8) & 0xFF);
    drive_write_port(drive, 5, (lba >> 16) & 0xFF);
    drive_write_port(drive, 6, 0xE0 | ((lba >> 24) & 0x0F) | drive.drive_select);

    // Send write command
    drive_write_port(drive, 7, 0x30);

    for (int sector = 0; sector < sector_count; sector++) {
        // Wait for drive to be ready
        if (!wait_for_drive(drive, 0x08, 0x08, 1000)) {
            return false;
        }

        // Write data
        for (int i = 0; i < 256; i++) {
            uint16_t data = buffer[sector*512 + i*2] | (buffer[sector*512 + i*2 + 1] << 8);
            drive_write_port(drive, 0, data);
        }
    }

    // Wait for write to complete
    return wait_for_drive(drive, 0x80, 0x00, 1000);
}

uint32_t ACPI::get_drive_size(const DriveInfo& drive) const {
    return drive.size_in_sectors * 512;  // Return size in bytes
}

void ACPI::send_ata_command(const DriveInfo& drive, uint8_t command, uint8_t features, uint8_t sector_count, uint32_t lba) {
    drive_write_port(drive, 6, 0xE0 | ((lba >> 24) & 0x0F));
    drive_write_port(drive, 1, features);
    drive_write_port(drive, 2, sector_count);
    drive_write_port(drive, 3, lba & 0xFF);
    drive_write_port(drive, 4, (lba >> 8) & 0xFF);
    drive_write_port(drive, 5, (lba >> 16) & 0xFF);
    drive_write_port(drive, 7, command);
}



bool ACPI::is_drive_ready(const DriveInfo& drive) const {
    uint8_t status = drive_read_port(drive, 7);
    bool bsy = (status & 0x80) != 0;  // Busy
    bool drdy = (status & 0x40) != 0; // Device Ready
    bool df = (status & 0x20) != 0;   // Device Fault
    bool drq = (status & 0x08) != 0;  // Data Request
    bool err = (status & 0x01) != 0;  // Error

    char log_message[100];
    format_string(log_message, sizeof(log_message), 
        "Drive status: 0x%02X ",
        status);
    Logger::info(log_message);

    return drdy && !bsy && !df && !err;  // Ready when DRDY is set, and BSY, DF, ERR are clear
}




void ACPI::send_drive_command(const DriveInfo& drive, uint8_t command) {
    drive_write_port(drive, 7, command);
}