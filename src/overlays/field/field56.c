#include "common.h"
#include "cdrom.h"
#include "akao.h"

#define SEQ_BLOB_BASE 0x80180000

extern void akao_cmd_f1(void);
extern AkaoHeader *D_8011F304;

s32 func_800A35F4(s32 arg0)
{
    akao_cmd_f1();
    D_8011F304 = (AkaoHeader *) 0x8013C000;
    cdrom_queue_read((arg0 + 0x8E) & 0xFFFF, (void *) 0x8013C000);
    cdrom_wait_queue_empty();
    return akao_register_bank(D_8011F304);
}

void func_800A3654(void)
{
    cdrom_queue_read(0x16, (void *)SEQ_BLOB_BASE);
    cdrom_wait_queue_empty();
    akao_upload_bank_blocking((AkaoBankHeader *)SEQ_BLOB_BASE, 1);
}
