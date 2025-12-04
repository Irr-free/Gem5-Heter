# Skeleton NPU SimObject definition (copied from gem5 accel demo).
from m5.params import *
from m5.objects.Device import BasicPioDevice


class NPU(BasicPioDevice):
    type = "NPU"
    cxx_header = "dev/accel/NPU.hh"
    cxx_class = "gem5::NPU"

    pio_addr = Param.Addr("Base address for NPU control block")
    pio_size = Param.Addr(0x40, "Size of NPU control register window")

    scratchpad_base = Param.Addr(
        0x10020080, "Base address of scratchpad region visible to the NPU"
    )

    compute_ticks = Param.Tick(1000, "Nominal compute latency in ticks")
    status_offset = Param.Addr(0x04, "Offset of the status register")
