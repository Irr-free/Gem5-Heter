/*
 * Skeleton NPU device that models a MMIO-controlled compute block.
 */

#include "dev/accel/NPU.hh"

#include "base/logging.hh"
#include "base/trace.hh"
#include "debug/NPU.hh"
#include "mem/packet_access.hh"
#include "sim/system.hh"

namespace gem5
{

NPU::NPU(const Params &p)
    : BasicPioDevice(p, p.pio_size),
      scratchpadBase(p.scratchpad_base),
      computeTicks(p.compute_ticks),
      computeEvent([this]() { completeCompute(); }, name())
{
}

Tick
NPU::read(PacketPtr pkt)
{
    const Addr offset = pkt->getAddr() - pioAddr;
    DPRINTF(NPU, "MMIO read offset %#x size %u\n", offset, pkt->getSize());

    switch (offset) {
      case kStatusReg:
        pkt->setLE<uint32_t>(status);
        break;
      case kInputAddrReg:
        pkt->setLE<uint64_t>(inputAddr);
        break;
      case kLengthReg:
        pkt->setLE<uint32_t>(length);
        break;
      default:
        pkt->setLE<uint64_t>(0);
        warn_once("NPU read from undefined offset %#x\n", offset);
        break;
    }

    pkt->makeResponse();
    return pioDelay;
}

Tick
NPU::write(PacketPtr pkt)
{
    const Addr offset = pkt->getAddr() - pioAddr;
    DPRINTF(NPU, "MMIO write offset %#x size %u\n", offset, pkt->getSize());

    switch (offset) {
      case kStartReg:
        triggerCompute();
        break;
      case kInputAddrReg:
        inputAddr = pkt->getLE<uint64_t>();
        break;
      case kLengthReg:
        length = pkt->getLE<uint32_t>();
        break;
      default:
        warn_once("NPU write to undefined offset %#x\n", offset);
        break;
    }

    pkt->makeResponse();
    return pioDelay;
}

void
NPU::triggerCompute()
{
    if (busy) {
        DPRINTF(NPU, "Compute request ignored (busy)\n");
        return;
    }

    busy = true;
    status = 0;

    schedule(computeEvent, curTick() + computeTicks);
    DPRINTF(NPU, "Compute scheduled for %llu ticks later\n",
            static_cast<unsigned long long>(computeTicks));
}

void
NPU::completeCompute()
{
    busy = false;
    status = 1;
    DPRINTF(NPU, "Compute completed; status set to ready\n");
}

} // namespace gem5
