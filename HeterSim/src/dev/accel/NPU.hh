/*
 * Skeleton 2D systolic-array style NPU model.
 */

#ifndef __DEV_ACCEL_NPU_HH__
#define __DEV_ACCEL_NPU_HH__

#include "dev/io_device.hh"
#include "mem/packet.hh"
#include "params/NPU.hh"
#include "sim/eventq.hh"

namespace gem5
{

class NPU : public BasicPioDevice
{
  public:
    using Params = NPUParams;
    NPU(const Params &p);

    Tick read(PacketPtr pkt) override;
    Tick write(PacketPtr pkt) override;

  private:
    static constexpr Addr kStartReg = 0x00;
    static constexpr Addr kStatusReg = 0x04;
    static constexpr Addr kInputAddrReg = 0x08;
    static constexpr Addr kLengthReg = 0x10;

    const Addr scratchpadBase;
    const Tick computeTicks;

    Addr inputAddr = 0;
    uint32_t length = 0;
    bool busy = false;
    uint32_t status = 0;

    EventFunctionWrapper computeEvent;

    void triggerCompute();
    void completeCompute();
};

} // namespace gem5

#endif // __DEV_ACCEL_NPU_HH__
