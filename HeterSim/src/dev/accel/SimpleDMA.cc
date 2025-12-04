/*
 * Skeleton non-coherent DMA engine.
 */

#include "dev/accel/SimpleDMA.hh"

#include <algorithm>
#include <vector>

#include "base/addr_range.hh"
#include "base/logging.hh"
#include "base/trace.hh"
#include "debug/SimpleDMA.hh"
#include "mem/packet_access.hh"

namespace gem5
{

SimpleDMA::SimpleDMA(const Params &p)
    : DmaDevice(p),
      pioAddr(p.pio_addr), pioSize(p.pio_size),
      pioLatency(p.pio_latency), maxTransfer(p.max_transfer_size),
      dmaEvent([this]() { finishDma(); }, name())
{
}

AddrRangeList
SimpleDMA::getAddrRanges() const
{
    return { RangeSize(pioAddr, pioSize) };
}

Tick
SimpleDMA::read(PacketPtr pkt)
{
    const Addr offset = pkt->getAddr() - pioAddr;
    DPRINTF(SimpleDMA, "MMIO read offset %#x size %u\n",
            offset, pkt->getSize());

    switch (offset) {
      case kSrcReg:
        pkt->setLE<uint64_t>(src);
        break;
      case kDstReg:
        pkt->setLE<uint64_t>(dst);
        break;
      case kLenReg:
        pkt->setLE<uint32_t>(len);
        break;
      case kStatusReg: {
          uint32_t bits = 0;
          bits |= busy ? 0x1 : 0x0;
          bits |= done ? 0x2 : 0x0;
          pkt->setLE<uint32_t>(bits);
        }
        break;
      default:
        pkt->setLE<uint64_t>(0);
        warn_once("SimpleDMA read from undefined offset %#x\n", offset);
        break;
    }

    pkt->makeResponse();
    return pioLatency;
}

Tick
SimpleDMA::write(PacketPtr pkt)
{
    const Addr offset = pkt->getAddr() - pioAddr;
    DPRINTF(SimpleDMA, "MMIO write offset %#x size %u\n",
            offset, pkt->getSize());

    switch (offset) {
      case kSrcReg:
        src = pkt->getLE<uint64_t>();
        break;
      case kDstReg:
        dst = pkt->getLE<uint64_t>();
        break;
      case kLenReg:
        len = pkt->getLE<uint32_t>();
        break;
      case kGoReg:
        startDma();
        break;
      case kStatusReg:
        done = false;
        break;
      default:
        warn_once("SimpleDMA write to undefined offset %#x\n", offset);
        break;
    }

    pkt->makeResponse();
    return pioLatency;
}

void
SimpleDMA::startDma()
{
    if (busy) {
        DPRINTF(SimpleDMA, "DMA already in progress\n");
        return;
    }

    if (len == 0) {
        done = true;
        return;
    }

    busy = true;
    done = false;
    phase = Phase::Reading;

    activeLen = std::min<unsigned>(len, maxTransfer);
    stagingBuf.resize(activeLen);
    if (len > maxTransfer) {
        warn_once("SimpleDMA transfer length %u exceeds max %u; truncating\n",
                  len, maxTransfer);
    }

    dmaRead(src, activeLen, &dmaEvent, stagingBuf.data());

    DPRINTF(SimpleDMA, "DMA read issued src=%#x dst=%#x len=%u\n",
            src, dst, len);
}

void
SimpleDMA::finishDma()
{
    if (!busy) {
        return;
    }

    if (phase == Phase::Reading) {
        phase = Phase::Writing;
        dmaWrite(dst, activeLen, &dmaEvent, stagingBuf.data());
        DPRINTF(SimpleDMA, "DMA write issued dst=%#x len=%u\n",
                dst, activeLen);
        return;
    }

    phase = Phase::Idle;
    busy = false;
    done = true;

    DPRINTF(SimpleDMA, "DMA completed src=%#x dst=%#x len=%u\n",
            src, dst, activeLen);
}

} // namespace gem5
