// cfg_flash.h — 用板上 flash 的最後兩個 4 KB 區塊保存設定（排程 + 排程開關）
//
// 配置（位移從 flash 開頭算起，用 PICO_FLASH_SIZE_BYTES 推算，換板子會跟著變）：
//   PRIMARY   = 倒數第 2 個 4 KB 區塊：正式存放設定的地方，開機從這裡讀
//   SECONDARY = 最後 1 個 4 KB 區塊：read-modify-write 的暫存區，平常保持空白
//
// 寫入順序（cfg_flash_save）：
//   1. 在 RAM 組好新的 256 byte 紀錄
//   2. 寫進 SECONDARY        ← 這步完成後，資料已經「落地」，斷電也還在
//   3. 寫進 PRIMARY
//   4. 抹除 SECONDARY        ← 正常結束後暫存區是空的
//
// 開機（cfg_flash_boot）：
//   SECONDARY 有有效資料 → 代表上次寫到一半斷電，把它補寫進 PRIMARY，再抹除 SECONDARY
//   否則讀 PRIMARY
//   兩邊都沒有（全新板子）→ 回報「沒有資料」，由呼叫端沿用程式裡的預設值
#ifndef _CFG_FLASH_H_
#define _CFG_FLASH_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CFG_SCH_RLY_SIZE 96   // gSCH_RLY[24*4]

typedef struct {
    uint8_t sch_en;                        // 排程開關：1 = 啟用、0 = 停用
    uint8_t sch_rly[CFG_SCH_RLY_SIZE];     // 排程表（4 個 relay × 24 小時）
} cfg_data_t;

// 開機時呼叫一次（含上次中斷的復原）。
//   回傳  0 = 從 PRIMARY 讀到設定，*out 已填好
//         1 = 從 SECONDARY 復原（上次寫入被中斷），已補寫進 PRIMARY，*out 已填好
//        -1 = 沒有有效設定（全新板子），*out 不變，請沿用預設值
int cfg_flash_boot(cfg_data_t *out);

// 存一次設定。寫入期間會關中斷（分段關閉，每段數十 ms），所以只在設定真的變更時呼叫。
//   回傳 0 = 成功；-1 = 寫入後驗證失敗
int cfg_flash_save(const cfg_data_t *in);

#ifdef __cplusplus
}
#endif

#endif
