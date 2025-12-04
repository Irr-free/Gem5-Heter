# Skeleton DMA SimObject for accelerator bring-up.
from m5.params import *
from m5.objects.Device import DmaDevice


class SimpleDMA(DmaDevice):
    type = "SimpleDMA"
    cxx_header = "dev/accel/SimpleDMA.hh"
    cxx_class = "gem5::SimpleDMA"

    pio_addr = Param.Addr("Base address for DMA registers")
    pio_size = Param.Addr(0x40, "Size of the DMA MMIO window")
    pio_latency = Param.Latency("50ns", "Latency for register accesses")

    max_transfer_size = Param.Unsigned(256, "Max bytes per issued DMA burst")
