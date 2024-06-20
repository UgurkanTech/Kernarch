# Kernel Boot Order:

1. GRUB
    - Input: None
    - Output: Control passed to Assembly Entry Point
    - Operations:
        - $ Loads the kernel into memory
        - $ Switches the CPU into protected mode
        - $ Jump to _start in ASM

2. Assembly Entry Point
    - Input: Control from GRUB
    - Output: Control passed to C Entry Point
    - Operations:
        - @ dd multiboot_header: Multiboot header
        - @ label stack_top: Stack top pointer
        - $ proc initialize_stack(): Sets up stack

3. C Entry Point
    - Input: Control from Assembly Entry Point
    - Output: Control passed to GDT Initialization
    - Operations:
        - $ call Jump to `void kmain(void)`: Begins execution in C

4. GDT Initialization
    - Input: Control from C Entry Point
    - Output: Control passed to Paging Initialization
    - Operations:
        - @ GDTEntry* gdt_entries: GDT entries array
        - $ void init_gdt(void): Initializes the Global Descriptor Table
            - $ void define_gdt_entries(GDTEntry* gdt_entries): Defines GDT entries
            - $ void setup_descriptors(void): Sets up CODE_SEG and DATA_SEG
            - $ void load_gdt(GDTEntry* gdt_entries): Loads GDT entries into CPU

5. Paging Initialization
    - Input: Control from GDT Initialization
    - Output: Control passed to IDT Initialization
    - Operations:
        - @ PageDirectory* page_directory: Page directory storage
        - @ PageTable* page_tables: Page tables array
        - $ void init_paging(void): Initializes memory paging
            - $ PageDirectory* allocate_page_directory(void): Allocates memory for page directory
            - $ PageTable* allocate_page_tables(void): Allocates memory for page tables
            - $ void setup_page_directory_entries(PageDirectory* page_directory): Sets up page directory entries
            - $ void setup_page_table_entries(PageTable* page_tables): Sets up page table entries
            - $ void enable_paging(void): Enables paging in processor
            - $ void load_page_directory(PageDirectory* page_directory): Loads page directory into `cr3` register

6. IDT Initialization
    - Input: Control from Paging Initialization
    - Output: Control passed to PIC Initialization
    - Operations:
        - @ IDTEntry* idt_entries: IDT entries array
        - $ void init_idt(void): Initializes the Interrupt Descriptor Table
            - $ void define_idt_entries(IDTEntry* idt_entries): Defines IDT entries
            - $ void setup_interrupt_gate_descriptors(void): Sets up INTERRUPT_GATE descriptors
            - $ void load_idt(IDTEntry* idt_entries): Loads IDT into CPU

7. PIC Initialization
    - Input: Control from IDT Initialization
    - Output: Control passed to ISR Initialization
    - Operations:
        - $ void init_pic(void): Initializes the Programmable Interrupt Controller
            - $ void remap_pics(void): Remaps PICs to avoid conflicts
            - $ void set_icw(void): Sets ICW1, ICW2, ICW3, and ICW4
            - $ void set_imr(void): Sets IMR to enable/disable interrupts
            - $ void send_eoi(void): Sends EOI command after handling interrupt

8. ISR Initialization
    - Input: Control from PIC Initialization
    - Output: ISR initialized
    - Operations:
        - @ ISRRoutine* isr_routines: ISR routines array
        - $ void init_isr(void): Initializes the Interrupt Service Routines
            - $ void define_isr_routines(ISRRoutine* isr_routines): Defines ISR routines
            - $ void register_isr_routines(ISRRoutine* isr_routines): Registers ISR routines
            - $ void enable_isr(void): Enables specific hardware interrupts

9. Other Initializations (Later)
    - Operations:
        - $ 9.1 File System Initialization
        - $ 9.2 Task Scheduler Initialization
        - $ 9.3 Network Driver Initialization
        - $ 9.4 Memory Management Initialization
        - $ 9.5 User Mode Environment Setup
        - $ 9.6 And more...

10. Main Kernel Loop
    - Operations:
        - $ Continuously checks for and handles system events

# Symbol Table

| Symbol | Description |
|--------|-------------|
| `-`    | Bullet points for operations, variables, and methods/functions |
| `@`    | Variables |
| `$`    | Methods/Functions |