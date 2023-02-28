#include <inc/assert.h>
#include <inc/error.h>
#include <inc/uefi.h>
#include <inc/x86.h>

#include <kern/traceopt.h>
#include <kern/pci.h>

void init_pci(void)
{
    if (trace_pci)
        cprintf("PCI initialization started. \n");

    int count = enumerate_pci_devices();

    if (trace_pci)
        cprintf("Enumerating PCI devices. Total count=%d \n", count);

    if (trace_pci)
        cprintf("PCI initialization successfully finished. \n");

    return; 
}

void dump_pci_dev(const pci_dev_t* pci_dev)
{
    assert(pci_dev != 0);
    cprintf("PCI_dev dump: \n");

    cprintf("BUS: 0x%02x DEVICE: 0x%02x FUNCTION: 0x%02x \n", pci_dev->bus_number, 
                                                              pci_dev->device_number,
                                                              pci_dev->function_number);
    cprintf("VENDORID 0x%04x DEVICEID 0x%04x \n",             pci_dev->vendor_id,
                                                              pci_dev->device_id);
    cprintf("REVISIONID 0x%02x PROGIF 0x%02x \n",             pci_dev->revision_id, 
                                                              pci_dev->prog_if);
    cprintf("CLASS 0x%02x SUBCLASS 0x%02x \n",                pci_dev->class_code, 
                                                              pci_dev->subclass);
    cprintf("CACHELINESIZE 0x%02x LATENCYTIMER 0x%02x \n",    pci_dev->cache_line_size, 
                                                              pci_dev->latency_timer);
    cprintf("HEADERTYPE 0x%02x \n",                           pci_dev->header_type);

    return;
}

void dump_pci_dev_general(const pci_dev_general_t* pci_dev_general)
{
    assert(pci_dev_general != 0);
    cprintf("PCI_dev_general dump: \n");

    dump_pci_dev(&(pci_dev_general->pci_dev));

    cprintf("BAR0 0x%04x BAR1 0x%04x BAR2 0x%04x BAR3 0x%04x BAR4 0x%04x BAR5 0x%04x\n", 
                                                                  pci_dev_general->BAR0,
                                                                  pci_dev_general->BAR1,
                                                                  pci_dev_general->BAR2,
                                                                  pci_dev_general->BAR3,
                                                                  pci_dev_general->BAR4,
                                                                  pci_dev_general->BAR5);

    cprintf("CARDBUS_CIS_PTR 0x%04x \n", pci_dev_general->cardbus_cis_ptr);

    cprintf("SUBSYSTEMVENDORID 0x%02x SUBSYSTEDEVICEMID 0x%02x \n", pci_dev_general->subsystem_vendor_id,
                                                                    pci_dev_general->subsystem_dev_id);

    cprintf("EXPANSIONBASEROMADDR 0x%04x \n", pci_dev_general->expansion_rom_base_addr);
    cprintf("CAPABILITIESPTR 0x%02x \n", pci_dev_general->capabilites_ptr);

    cprintf("INTLINE 0x%02x INTPIN 0x%02x MIN_GRANT 0x%02x MAX_LAT 0x%02x \n", pci_dev_general->interrupt_line,
                                                                               pci_dev_general->interrupt_pin,
                                                                               pci_dev_general->min_grant,
                                                                               pci_dev_general->max_latency);

    return;
}

/* PCI device opeartions */

uint16_t pci_dev_get_stat_reg(const pci_dev_t* pci_dev)
{
    assert(pci_dev != 0);
    return pci_config_read16(pci_dev, PCI_CONF_SPACE_STATUS);
}

void pci_dev_set_cmnd_reg(const pci_dev_t* pci_dev, uint16_t val)
{
    assert(pci_dev != 0);

    pci_config_write16(pci_dev, PCI_CONF_SPACE_COMMAND, val);
    return;
}

uint16_t pci_dev_get_cmnd_reg(const pci_dev_t* pci_dev)
{
    assert(pci_dev != 0);
    return pci_config_read16(pci_dev, PCI_CONF_SPACE_COMMAND);
}

int pci_dev_find(pci_dev_t* pci_dev, uint16_t class, uint16_t subclass, uint16_t vendor_id)
{
    assert(pci_dev != 0);

    uint16_t bus      = 0;
    uint8_t  dev      = 0;
    uint8_t  function = 0;

    bool found = false;

    for (bus = 0; bus < MAX_BUS; bus++)
    {
        for (dev = 0; dev < MAX_DEV; dev++)
        {
            function = 0;

            if (check_pci_device(bus, dev, function) == false)
                continue;

            cprintf("class 0x%x subclass 0x%x \n", get_class   (bus, dev, function), 
                                                   get_subclass(bus, dev, function));

            if (class     == get_class    (bus, dev, function)
             && subclass  == get_subclass (bus, dev, function)
             && vendor_id == get_vendor_id(bus, dev, function))
            {
                found = true;
                goto found;
            }

            bool is_mf = check_multi_function(bus, dev);
            if (is_mf)
            {
                for (function = 1; function < MAX_FUN; function++)
                {
                    if (check_pci_device(bus, dev, function) == false)
                        continue;

                    cprintf("class 0x%x subclass 0x%x \n", get_class   (bus, dev, function), 
                                                           get_subclass(bus, dev, function));

                    if (class     == get_class    (bus, dev, function)
                     && subclass  == get_subclass (bus, dev, function)
                     && vendor_id == get_vendor_id(bus, dev, function))
                    {
                        found = true;
                        goto found;
                    }
                }
            }
        }
    }

found:

    if (found == false)
        return -1;

    pci_dev->bus_number      = (uint8_t) bus;
    pci_dev->device_number   = dev;
    pci_dev->function_number = function;

    pci_dev->class_code = class;
    pci_dev->subclass   = subclass;
    pci_dev->vendor_id  = vendor_id;

    return 0;
}

int pci_dev_read_header(pci_dev_t* pci_dev)
{
    assert(pci_dev != 0);

    if (check_pci_device(pci_dev->bus_number, pci_dev->device_number, pci_dev->function_number) == false)
        return -1;

    pci_dev->device_id       = pci_config_read16(pci_dev, PCI_CONF_SPACE_DEVICE_ID);

    pci_dev->revision_id     = pci_config_read16(pci_dev, PCI_CONF_SPACE_REVISION_ID);
    pci_dev->prog_if         = pci_config_read16(pci_dev, PCI_CONF_SPACE_PROG_IF);

    pci_dev->cache_line_size = pci_config_read16(pci_dev, PCI_CONF_SPACE_CACHE_LINE_SIZE);
    pci_dev->latency_timer   = pci_config_read16(pci_dev, PCI_CONF_SPACE_LATENCY_TIMER);
    pci_dev->header_type     = pci_config_read16(pci_dev, PCI_CONF_SPACE_HEADER_TYPE);

    return 0;
}

int pci_dev_general_read_header(pci_dev_general_t* pci_dev_general)
{
    assert(pci_dev_general != 0);
    pci_dev_t* pci_dev = &(pci_dev_general->pci_dev);

    if (pci_dev_read_header(pci_dev) == -1)
        return -1;
    
    pci_dev_general->BAR0 = pci_config_read32(pci_dev, PCI_CONF_SPACE_TYPE_0_BAR0); 
    pci_dev_general->BAR1 = pci_config_read32(pci_dev, PCI_CONF_SPACE_TYPE_0_BAR1);
    pci_dev_general->BAR2 = pci_config_read32(pci_dev, PCI_CONF_SPACE_TYPE_0_BAR2);
    pci_dev_general->BAR3 = pci_config_read32(pci_dev, PCI_CONF_SPACE_TYPE_0_BAR3);
    pci_dev_general->BAR4 = pci_config_read32(pci_dev, PCI_CONF_SPACE_TYPE_0_BAR4);
    pci_dev_general->BAR5 = pci_config_read32(pci_dev, PCI_CONF_SPACE_TYPE_0_BAR5);

    pci_dev_general->cardbus_cis_ptr = pci_config_read32(pci_dev, PCI_CONF_SPACE_TYPE_0_CARDBUS_CIS);
    
    pci_dev_general->subsystem_vendor_id = pci_config_read16(pci_dev, PCI_CONF_SPACE_TYPE_0_SUB_VENDOR_ID);
    pci_dev_general->subsystem_dev_id    = pci_config_read16(pci_dev, PCI_CONF_SPACE_TYPE_0_SUB_DEVICE_ID);

    pci_dev_general->expansion_rom_base_addr = pci_config_read32(pci_dev, PCI_CONF_SPACE_TYPE_0_EXPANSION_ROM);

    pci_dev_general->capabilites_ptr = pci_config_read8(pci_dev, PCI_CONF_SPACE_TYPE_0_CAPABILITIES); 

    pci_dev_general->interrupt_line = pci_config_read8(pci_dev, PCI_CONF_SPACE_TYPE_0_INTERRUPT_LINE);
    pci_dev_general->interrupt_pin  = pci_config_read8(pci_dev, PCI_CONF_SPACE_TYPE_0_INTERRUPT_PIN);
    pci_dev_general->min_grant      = pci_config_read8(pci_dev, PCI_CONF_SPACE_TYPE_0_MIN_GNT);
    pci_dev_general->max_latency    = pci_config_read8(pci_dev, PCI_CONF_SPACE_TYPE_0_MAX_LAT);

    return 0;
}

/* Enumerating PCI buses */

uint8_t get_header_type (uint8_t bus, uint8_t dev)
{
    pci_dev_t device = { 0 };

    device.bus_number      = bus;
    device.device_number   = dev;
    device.function_number = 0;

    return pci_config_read8(&device, PCI_CONF_SPACE_HEADER_TYPE);
}

bool check_multi_function(uint8_t bus, uint8_t dev)
{
    return (get_header_type(bus, dev) & HEADER_TYPE_REG_MF);
}

uint16_t get_vendor_id(uint8_t bus, uint8_t dev, uint8_t function)
{
    pci_dev_t device = { 0 };

    device.bus_number      = bus;
    device.device_number   = dev;
    device.function_number = function;

    return pci_config_read16(&device, PCI_CONF_SPACE_VENDOR_ID);
}

uint8_t get_class(uint8_t bus, uint8_t dev, uint8_t function)
{
    pci_dev_t device = { 0 };

    device.bus_number      = bus;
    device.device_number   = dev;
    device.function_number = function;

    return pci_config_read8(&device, PCI_CONF_SPACE_CLASS);
}

uint8_t get_subclass(uint8_t bus, uint8_t dev, uint8_t function)
{
    pci_dev_t device = { 0 };

    device.bus_number      = bus;
    device.device_number   = dev;
    device.function_number = function;

    return pci_config_read8(&device, PCI_CONF_SPACE_SUBCLASS);
}

bool check_pci_device(uint8_t bus, uint8_t device, uint8_t function)
{
    uint16_t vendor_id = get_vendor_id(bus, device, function);
    bool present = (vendor_id != NON_EXISTING_VENDOR_ID);

    if (trace_pci_more && present)
    {
        cprintf("Checking device at: bus=0x%03x device=0x%02x function=0x%x. ", 
                                                        bus, device, function);
        cprintf("Vendor_id=0x%04x. \n", vendor_id);
    }

    return present;
}


// performs Brute Force scan
int enumerate_pci_devices(void) 
{
    int count = 0;

    for (uint16_t bus = 0; bus < MAX_BUS; bus++)
    {
        for (uint8_t dev = 0; dev < MAX_DEV; dev++)
        {
            uint8_t function = 0;

            if (check_pci_device(bus, dev, function) == false)
                continue;

            count++;

            bool is_mf = check_multi_function(bus, dev);

            if (trace_pci_more)
                cprintf("Device(bus=0x%03x device=0x%02x function=0x%x) is MF: %s. \n", 
                                             bus, dev, function, (is_mf)? "yes": "no");                

            if (is_mf)
            {
                for (function = 1; function < MAX_FUN; function++)
                {
                    if (check_pci_device(bus, dev, function) == true)
                        count++;
                }
            }
        }
    }

    return count;
}

/* Read-write functions for PIC configuration space */

uint8_t pci_config_read8(const pci_dev_t* pci_dev, uint8_t offset)
{
    assert(pci_dev != 0);
    return (uint8_t) (pci_config_read32(pci_dev, offset) >> ((offset & 0x3) * 8) & 0xFF);
}


uint16_t pci_config_read16(const pci_dev_t* pci_dev, uint8_t offset)
{
    assert(pci_dev != 0);
    return (uint16_t) (pci_config_read32(pci_dev, offset) >> ((offset & 0x2) * 8) & 0xFFFF);
}

uint32_t pci_config_read32(const pci_dev_t* pci_dev, uint8_t offset)
{
    assert(pci_dev != 0);

    uint32_t addr = PCI_CONF_ADDR_PORT_ENABLE_BIT 
                  | ((uint32_t) pci_dev->bus_number)      << PCI_CONF_ADDR_PORT_BUS_OFFS
                  | ((uint32_t) pci_dev->device_number)   << PCI_CONF_ADDR_PORT_DEV_OFFS
                  | ((uint32_t) pci_dev->function_number) << PCI_CONF_ADDR_PORT_FUN_OFFS
                  | (offset & PCI_REGISTER_OFFS_MASK);

    uint32_t prev = inl(PCI_CONFIGURATION_ADDR_PORT);
    outl(PCI_CONFIGURATION_ADDR_PORT, addr);

    uint32_t val = inl(PCI_CONFIGURATION_DATA_PORT);

    outl(PCI_CONFIGURATION_ADDR_PORT, prev);
    return val;
}

void pci_config_write8(const pci_dev_t* pci_dev, uint8_t offset, uint8_t val)
{
    assert(pci_dev != 0);

    uint16_t current = pci_config_read16(pci_dev, offset);
    uint16_t new     = ((uint16_t)val << ((offset & 0x1) * 8)) | (current & (0xFF00 >> (offset & 0x1) * 8));

    pci_config_write16(pci_dev, offset, new);
}

void pci_config_write16(const pci_dev_t* pci_dev, uint8_t offset, uint16_t val)
{
    assert(pci_dev != 0);

    uint32_t current = pci_config_read32(pci_dev, offset);
    uint32_t new     = ((uint32_t)val << ((offset & 0x2) * 8)) | (current & (0xFFFF0000 >> (offset & 0x2) * 8));

    pci_config_write32(pci_dev, offset, new);
}

void pci_config_write32(const pci_dev_t* pci_dev, uint8_t offset, uint32_t val)
{
    assert(pci_dev != 0);

    uint32_t addr = PCI_CONF_ADDR_PORT_ENABLE_BIT 
                  | ((uint32_t) pci_dev->bus_number)      << PCI_CONF_ADDR_PORT_BUS_OFFS
                  | ((uint32_t) pci_dev->device_number)   << PCI_CONF_ADDR_PORT_DEV_OFFS
                  | ((uint32_t) pci_dev->function_number) << PCI_CONF_ADDR_PORT_FUN_OFFS
                  | (offset & PCI_REGISTER_OFFS_MASK);

    uint32_t prev = inl(PCI_CONFIGURATION_ADDR_PORT);
    outl(PCI_CONFIGURATION_ADDR_PORT, addr);

    outl(PCI_CONFIGURATION_DATA_PORT, val);
    outl(PCI_CONFIGURATION_ADDR_PORT, prev);

    return;
}