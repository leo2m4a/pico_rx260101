// cfg_flash.c — 設定保存在板上 flash（詳見 cfg_flash.h 的配置說明）
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

#include "cfg_flash.h"

// ---- 區塊位置 ----
#define CFG_PRI_OFFSET   (PICO_FLASH_SIZE_BYTES - 2u * FLASH_SECTOR_SIZE)
#define CFG_SEC_OFFSET   (PICO_FLASH_SIZE_BYTES - 1u * FLASH_SECTOR_SIZE)

// ---- 紀錄格式（固定 1 頁 = 256 bytes，flash 寫入的最小單位）----
//   [0..3]   magic "SCFG"
//   [4]      version
//   [5]      保留
//   [6..7]   payload 長度（little endian）
//   [8..9]   payload 的 CRC-16/CCITT-FALSE（little endian）
//   [10..11] 保留
//   [12..]   payload（cfg_data_t）
#define CFG_HDR_SIZE   12
#define CFG_VERSION    1
#define CFG_REC_SIZE   FLASH_PAGE_SIZE

static const uint8_t k_magic[4] = { 'S', 'C', 'F', 'G' };

static uint16_t crc16_ccitt(const uint8_t *d, uint32_t n)
{
    uint16_t crc = 0xffff;
    uint32_t i;
    int b;
    for (i = 0; i < n; i++) {
        crc ^= (uint16_t)d[i] << 8;
        for (b = 0; b < 8; b++) {
            crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
        }
    }
    return crc;
}

// flash 直接對應在記憶體位址上，讀取不需要特別的函式
static const uint8_t *blk(uint32_t offset)
{
    return (const uint8_t *)(XIP_BASE + offset);
}

// 檢查一個區塊開頭的紀錄是否有效，有效就把 payload 複製到 *out
static int rec_read(uint32_t offset, cfg_data_t *out)
{
    const uint8_t *p = blk(offset);
    uint16_t len, crc;

    if (memcmp(p, k_magic, sizeof k_magic) != 0) { return 0; }
    if (p[4] != CFG_VERSION)                     { return 0; }

    len = (uint16_t)(p[6] | ((uint16_t)p[7] << 8));
    crc = (uint16_t)(p[8] | ((uint16_t)p[9] << 8));
    if (len != sizeof(cfg_data_t))                      { return 0; }
    if (CFG_HDR_SIZE + len > CFG_REC_SIZE)              { return 0; }
    if (crc16_ccitt(p + CFG_HDR_SIZE, len) != crc)      { return 0; }

    memcpy(out, p + CFG_HDR_SIZE, len);
    return 1;
}

static void rec_build(uint8_t *rec, const cfg_data_t *in)
{
    uint16_t len = (uint16_t)sizeof(cfg_data_t);
    uint16_t crc;

    memset(rec, 0xff, CFG_REC_SIZE);          // 0xff = flash 抹除後的狀態
    memcpy(rec, k_magic, sizeof k_magic);
    rec[4] = CFG_VERSION;
    rec[5] = 0;
    rec[6] = (uint8_t)(len & 0xff);
    rec[7] = (uint8_t)(len >> 8);
    memcpy(rec + CFG_HDR_SIZE, in, len);
    crc = crc16_ccitt(rec + CFG_HDR_SIZE, len);
    rec[8]  = (uint8_t)(crc & 0xff);
    rec[9]  = (uint8_t)(crc >> 8);
    rec[10] = 0;
    rec[11] = 0;
}

// 抹除一個區塊再寫入一頁。
// RP2040 的程式是從 flash 執行（XIP）的，抹除／寫入期間不能有別的程式去讀 flash，
// 所以要關中斷：timer 和 radio 的中斷處理程式都在 flash 裡。
// 抹除和寫入分兩段各自關閉，縮短每次關中斷的時間（抹除 4 KB 約數十 ms）。
// 註：這裡假設只有一個核心在跑。之後若啟用 core1，要改用 multicore_lockout 系列 API。
static void blk_write(uint32_t offset, const uint8_t *rec)
{
    uint32_t irq_s;

    irq_s = save_and_disable_interrupts();
    flash_range_erase(offset, FLASH_SECTOR_SIZE);
    restore_interrupts(irq_s);

    irq_s = save_and_disable_interrupts();
    flash_range_program(offset, rec, CFG_REC_SIZE);
    restore_interrupts(irq_s);
}

static void blk_erase(uint32_t offset)
{
    uint32_t irq_s = save_and_disable_interrupts();
    flash_range_erase(offset, FLASH_SECTOR_SIZE);
    restore_interrupts(irq_s);
}

int cfg_flash_boot(cfg_data_t *out)
{
    cfg_data_t tmp;

    // SECONDARY 有資料 → 上次 cfg_flash_save() 沒跑完，接著把它寫進 PRIMARY
    if (rec_read(CFG_SEC_OFFSET, &tmp)) {
        uint8_t rec[CFG_REC_SIZE];
        rec_build(rec, &tmp);
        blk_write(CFG_PRI_OFFSET, rec);
        blk_erase(CFG_SEC_OFFSET);
        *out = tmp;
        return 1;
    }

    if (rec_read(CFG_PRI_OFFSET, &tmp)) {
        *out = tmp;
        return 0;
    }

    return -1;      // 全新板子：沒有有效設定
}

int cfg_flash_save(const cfg_data_t *in)
{
    uint8_t rec[CFG_REC_SIZE];
    cfg_data_t chk;

    rec_build(rec, in);

    blk_write(CFG_SEC_OFFSET, rec);     // 1. 先落地到暫存區
    blk_write(CFG_PRI_OFFSET, rec);     // 2. 再寫正式區

    if (!rec_read(CFG_PRI_OFFSET, &chk) ||
        memcmp(&chk, in, sizeof(cfg_data_t)) != 0) {
        return -1;                      // PRIMARY 驗證失敗：暫存區留著，下次開機會補
    }

    blk_erase(CFG_SEC_OFFSET);          // 3. 正常結束，清空暫存區
    return 0;
}
