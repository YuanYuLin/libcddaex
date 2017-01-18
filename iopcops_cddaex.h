#ifndef IOPCOPS_CDDAEX_H
#define IOPCOPS_CDDAEX_H

struct ops_cddaex_t {
    int (*read_cdid)(uint8_t* cdrom, uint32_t* cdid);
    uint8_t (*get_no_tracks)(uint8_t* cdrom);
    int (*convert2wavefile)(uint8_t* cdrom, uint8_t* wavefile, uint8_t track_num);
}

#endif
