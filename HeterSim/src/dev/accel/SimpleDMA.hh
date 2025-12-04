/*
 * Skeleton non-coherent DMA controller for accelerator data moves.
 */

#ifndef __DEV_ACCEL_SIMPLE_DMA_HH__
#define __DEV_ACCEL_SIMPLE_DMA_HH__

#include <vector>

#include "dev/dma_device.hh"
#include "mem/packet.hh"
#include "params/SimpleDMA.hh"
#include "sim/eventq.hh"

namespace gem5
{

class SimpleDMA : public DmaDevice
{
  public:
    using Params = SimpleDMAParams;
    SimpleDMA(const Params &p);

    Tick read(PacketPtr pkt) override;
    Tick write(PacketPtr pkt) override;
    AddrRangeList getAddrRanges() const override;

  private:
    enum class Phase { Idle, Reading, Writing };

    static constexpr Addr kSrcReg = 0x00;
    static constexpr Addr kDstReg = 0x08;
    static constexpr Addr kLenReg = 0x10;
    static constexpr Addr kGoReg = 0x14;
    static constexpr Addr kStatusReg = 0x18;

    const Addr pioAddr;
    const Addr pioSize;
    const Tick pioLatency;
    const unsigned maxTransfer;

    Addr src = 0;
    Addr dst = 0;
    uint32_t len = 0;
    bool busy = false;
    bool done = false;
    Phase phase = Phase::Idle;

    std::vector<uint8_t> stagingBuf;
    unsigned activeLen = 0;

    EventFunctionWrapper dmaEvent;

    void startDma();
    void finishDma();
};

} // namespace gem5

#endif // __DEV_ACCEL_SIMPLE_DMA_HH__
