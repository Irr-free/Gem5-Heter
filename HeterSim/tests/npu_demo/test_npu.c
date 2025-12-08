#include <stdint.h>

#define DMA_BASE       0x10020000
#define DMA_SRC        (*(volatile uint64_t *)(DMA_BASE + 0x00))
#define DMA_DST        (*(volatile uint64_t *)(DMA_BASE + 0x08))
#define DMA_LEN        (*(volatile uint32_t *)(DMA_BASE + 0x10))
#define DMA_GO         (*(volatile uint32_t *)(DMA_BASE + 0x14))
#define DMA_STATUS     (*(volatile uint32_t *)(DMA_BASE + 0x18))

#define NPU_BASE       0x10020040
#define NPU_START      (*(volatile uint32_t *)(NPU_BASE + 0x00))
#define NPU_STATUS     (*(volatile uint32_t *)(NPU_BASE + 0x04))

#define SPM_BASE       0x10020080

static inline void dma_copy(uint64_t src, uint64_t dst, uint32_t len)
{
    DMA_SRC = src;
    DMA_DST = dst;
    DMA_LEN = len;
    DMA_GO = 1;
    while (DMA_STATUS & 0x1) {
        /* busy-wait */
    }
}

int main(void)
{
    static uint32_t data[256];
    for (uint32_t i = 0; i < 256; ++i) {
        data[i] = i;
    }

    /* move data into scratchpad */
    dma_copy((uint64_t)data, SPM_BASE, sizeof(data));

    /* kick the NPU */
    NPU_START = 1;
    while (NPU_STATUS == 0) {
        /* busy-wait */
    }

    /* bring results back */
    dma_copy(SPM_BASE, (uint64_t)data, sizeof(data));

    /* verify */
    int errors = 0;
    for (uint32_t i = 0; i < 256; ++i) {
        if (data[i] != i) {
            ++errors;
        }
    }
    return errors;
}
