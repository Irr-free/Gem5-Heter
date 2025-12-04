# Minimal CPU+NPU+DMA+HBM/DRAM integration example using HeterSim assets.
import m5
from m5.objects import (
    AddrRange,
    DDR4_2400_8x8,
    HBM_1000_4H_1x128,
    MemCtrl,
    Root,
    RiscvTimingSimpleCPU,
    SimpleDMA,
    SimpleMemory,
    SrcClockDomain,
    System,
    SystemXBar,
    VoltageDomain,
    NPU,
)


def build_system():
    system = System()
    system.clk_domain = SrcClockDomain(
        clock="1GHz", voltage_domain=VoltageDomain()
    )
    system.mem_mode = "timing"

    system.mem_ranges = [
        AddrRange(0x80000000, size="512MB"),
        AddrRange(0xA0000000, size="1536MB"),
    ]

    system.membus = SystemXBar(width=16)
    system.acc_bus = SystemXBar(width=16)

    system.cpu = [
        RiscvTimingSimpleCPU(cpu_id=0),
        RiscvTimingSimpleCPU(cpu_id=1),
    ]
    for cpu in system.cpu:
        cpu.icache_port = system.membus.cpu_side_ports
        cpu.dcache_port = system.membus.cpu_side_ports
        cpu.createThreads()

    system.hbm_ctrl = MemCtrl(
        dram=HBM_1000_4H_1x128(),
        range=AddrRange(0x80000000, size="512MB"),
    )
    system.dram_ctrl = MemCtrl(
        dram=DDR4_2400_8x8(),
        range=AddrRange(0xA0000000, size="1536MB"),
    )
    system.hbm_ctrl.port = system.membus.mem_side_ports
    system.dram_ctrl.port = system.membus.mem_side_ports

    system.spmem = SimpleMemory(
        range=AddrRange(0x10020080, size="32kB"), latency="10ns"
    )
    system.spmem.port = system.acc_bus.mem_side_ports

    system.npu = NPU(
        pio_addr=0x10020040,
        pio_size=0x40,
        scratchpad_base=0x10020080,
        compute_ticks=1000,
    )
    system.dma = SimpleDMA(
        pio_addr=0x10020000,
        pio_size=0x40,
        max_transfer_size=256,
    )

    system.npu.pio = system.acc_bus.mem_side_ports
    system.dma.pio = system.acc_bus.mem_side_ports
    system.dma.dma = system.membus.cpu_side_ports

    system.membus.mem_side_ports = system.acc_bus.cpu_side_ports
    system.acc_bus.mem_side_ports = system.membus.cpu_side_ports
    system.acc_bus.range = AddrRange(0x10020000, 0x1002FFFF)

    system.system_port = system.membus.cpu_side_ports

    return system


if __name__ == "__m5_main__":
    system = build_system()
    root = Root(full_system=False, system=system)
    m5.instantiate()

    print("NPU hetero system instantiated; starting simulation.")
    exit_event = m5.simulate()
    print(
        f"Exited at tick {m5.curTick()} because {exit_event.getCause()}"
    )
