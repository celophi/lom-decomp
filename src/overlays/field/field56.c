#include "common.h"
#include "cdrom.h"
#include "akao.h"

#define SEQ_BLOB_BASE 0x80180000

void func_800A3654(void)
{
    cdrom_queue_read(0x16, (void *)SEQ_BLOB_BASE);
    cdrom_wait_queue_empty();
    akao_upload_bank_blocking((AkaoBankHeader *)SEQ_BLOB_BASE, 1);
}
