#include <stdio.h>

#include "iopcdefine.h"

struct riff_t {
  uint32_t file_tag;    //RIFF
  uint32_t file_size;
  uint32_t file_type;   //WAVE

  uint32_t fmt_tag;     //fmt
  uint32_t fmt_size;    //
  uint16_t fmt_code;
  uint16_t channels;
  uint32_t sample_rate;
  uint32_t data_rate;
  uint16_t data_block_size;
  uint16_t bit_per_sample;
  uint32_t chunk_tag;
  uint32_t data_size;
};

static int cddb_sum(int n)
{
    int result = 0;
    while (n > 0) {
        result += n % 10;
        n /= 10;
    }
    return result;
}

static int read_cdid(uint8_t* cdrom, uint32_t* cdid)
{
    CdIo_t *p_cdio = cdio_open (cdrom, DRIVER_UNKNOWN);
    track_t i_track;
    track_t i_tracks;
    uint32_t start_sec = 0;
    uint32_t leadout_sec = 0;
    uint32_t total = 0;
    uint32_t n = 0;

    if (NULL == p_cdio) {
        return -1;
    }

    i_tracks = cdio_get_num_tracks(p_cdio);
    for (i_track = 1; i_track <= i_tracks; ++i_track) {
        lba_t lba = cdio_get_track_lba(p_cdio, i_track);
        n += cddb_sum(lba / CDIO_CD_FRAMES_PER_SEC);
    }

    start_sec = cdio_get_track_lba(p_cdio, 1) /  CDIO_CD_FRAMES_PER_SEC;
    leadout_sec = cdio_get_track_lba(p_cdio, CDIO_CDROM_LEADOUT_TRACK) / CDIO_CD_FRAMES_PER_SEC;
    total = leadout_sec - start_sec;

    *cdid = ((n % 0xff) << 24 | total << 8 | i_tracks);

    cdio_destroy(p_cdio);

    return 0;
}

static uint8_t get_no_tracks(uint8_t* cdrom)
{
    CdIo_t *p_cdio = cdio_open (cdrom, DRIVER_UNKNOWN);
    track_t i_tracks;
    if (NULL == p_cdio) {
        return -1;
    }

    i_tracks = cdio_get_num_tracks(p_cdio);

    cdio_destroy(p_cdio);
    return i_tracks;
}

static int convert2wavefile(uint8_t* cdrom, uint8_t* wavefile, uint8_t track_num)
{
    int sectors = 0;
    int sector = 0;
    track_t track;
    uint8_t data[CDIO_CD_FRAMESIZE_RAW] = { 0 };
    struct riff_t riff;
    FILE *fp = NULL;
    int i = 0;
    CdIo_t *p_cdio = cdio_open (cdrom, DRIVER_UNKNOWN);
    if (NULL == p_cdio) {
        return -1;
    }

    track = cdio_get_first_track_num(p_cdio);
    for(i=0;i<track_num;i++) {
       if(i==track_num)
            break;
       track += 1;
    }

    lsn_t lsn_start = cdio_get_track_lsn(p_cdio, track);
    lsn_t lsn_end = cdio_get_track_lsn(p_cdio, track + 1);

    if (CDIO_INVALID_LSN == lsn_start) {
        return -1;
    }

    if (CDIO_INVALID_LSN == lsn_end) {
        return -1;
    }

    sectors = (lsn_end - lsn_start);

    fp = fopen(wavefile, "ab+");
    memset(&riff, 0, sizeof(struct riff_t));
    memcpy(&riff.file_tag, "RIFF", 4);
    riff.file_size = sectors * CDIO_CD_FRAMESIZE_RAW + 44 - 8;
    memcpy(&riff.file_type, "WAVE", 4);
    memcpy(&riff.fmt_tag, "fmt ", 4);
    riff.fmt_size = 16;
    riff.fmt_code = 1;
    riff.channels = 2;
    riff.sample_rate = 44100;
    riff.data_rate = 176400;
    riff.data_block_size = 4;
    riff.bit_per_sample = 16;
    memcpy(&riff.chunk_tag, "data", 4);
    riff.data_size = sectors * CDIO_CD_FRAMESIZE_RAW;
    fwrite(&riff, sizeof(struct riff_t), 1, fp);
    for(sector=lsn_start;sector<lsn_end;sector++) {
        cdio_read_audio_sector(p_cdio, &data, sector);
        fwrite(&data, CDIO_CD_FRAMESIZE_RAW, sizeof(unsigned char), fp);
    }
    fclose(fp);

    cdio_destroy(p_cdio);
    return 0;
}

//static struct ops_cddaex_t* obj = NULL;

DEFINE_INSTANCE(ops_cddaex);
DEFINE_GET_INSTANCE(ops_cddaex)
{
    if(!obj) {
        obj = malloc(sizeof(struct ops_cddaex_t));
	obj->read_cdid = read_cdid;
	obj->get_no_tracks = get_no_tracks;
	obj->convert2wavefile = convert2wavefile;
    }

    return obj;
}

DEFINE_DEL_INSTANCE(ops_cddaex)
{
    if(obj)
        free(obj);
}
