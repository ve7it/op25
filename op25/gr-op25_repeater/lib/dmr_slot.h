//
// DMR Protocol Decoder (C) Copyright 2019 Graham J. Norbury
// 
// This file is part of OP25
// 
// OP25 is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
// 
// OP25 is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
// License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with OP25; see the file COPYING. If not, write to the Free
// Software Foundation, Inc., 51 Franklin Street, Boston, MA
// 02110-1301, USA.

#ifndef INCLUDED_DMR_SLOT_H
#define INCLUDED_DMR_SLOT_H

#include <stdint.h>
#include <vector>

#include "frame_sync_magics.h"
#include "dmr_const.h"
#include "bptc19696.h"
#include "ezpwd/rs"

typedef std::vector<bool> bit_vector;
typedef std::vector<uint8_t> byte_vector;

static const unsigned int DMR_SYNC_THRESHOLD       = 6;
static const unsigned int DMR_SYNC_MAGICS_COUNT    = 9;
static const uint64_t     DMR_SYNC_MAGICS[]        = {DMR_BS_VOICE_SYNC_MAGIC,
						      DMR_BS_DATA_SYNC_MAGIC,
						      DMR_MS_VOICE_SYNC_MAGIC,
						      DMR_MS_DATA_SYNC_MAGIC,
						      DMR_MS_RC_SYNC_MAGIC,
						      DMR_T1_VOICE_SYNC_MAGIC,
						      DMR_T1_DATA_SYNC_MAGIC,
						      DMR_T2_VOICE_SYNC_MAGIC,
						      DMR_T2_DATA_SYNC_MAGIC};

static const uint8_t VOICE_LC_HEADER_CRC_MASK[]    = {1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0}; // 0x969696
static const uint8_t TERMINATOR_WITH_LC_CRC_MASK[] = {1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1}; // 0x999999
static const uint8_t PI_HEADER_CRC_MASK[]          = {0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1};                 // 0x6969
static const uint8_t DATA_HEADER_CRC_MASK[]        = {1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0};                 // 0xCCCC
static const uint8_t CSBK_CRC_MASK[]               = {1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1};                 // 0xA5A5

static const unsigned int SLOT_SIZE                = 264; // size in bits
static const unsigned int PAYLOAD_L                =   0; // starting position in bits
static const unsigned int PAYLOAD_R                = 156;
static const unsigned int SYNC_EMB                 = 108;
static const unsigned int SLOT_L                   =  98;
static const unsigned int SLOT_R                   = 156;

class dmr_slot {
public:
	dmr_slot(const int chan, const int debug = 0);
	~dmr_slot();
	inline void set_debug(const int debug) { d_debug = debug; };
	bool load_slot(const uint8_t slot[], uint64_t sl_type);

private:
	uint8_t     d_slot[SLOT_SIZE];	// array of bits comprising the current slot
	bit_vector  d_slot_type;
	byte_vector d_emb;
	byte_vector d_lc;		// last received LC data
	uint8_t     d_rc;		// last received RC data
	uint16_t    d_sb;		// last received SB data
	byte_vector d_pi;
	bool        d_lc_valid;		// flag indicating if LC data is valid
	bool        d_rc_valid;		// flag indicating if RC data is valid
	bool        d_sb_valid;		// flag indicating if SB data is valid
	uint64_t    d_type;
	uint8_t     d_cc;
	int         d_debug;
	int         d_chan;
	CBPTC19696  bptc;
	ezpwd::RS<255,252> rs12;	// Reed-Solomon(12,9) object for Link Control decode

	bool decode_slot_type();
	bool decode_csbk(uint8_t* csbk);
	bool decode_vlch(uint8_t* vlch);
	bool decode_tlc(uint8_t* tlc);
	bool decode_lc(uint8_t* lc, int* rs_errs = NULL);
	bool decode_pinf(uint8_t* pinf);
	bool decode_emb();
	bool decode_embedded_lc();
	bool decode_embedded_sbrc(bool _pi);

	inline uint8_t  get_lc_pf()      { return d_lc_valid ? ((d_lc[0] & 0x80) >> 7) : 0; };
	inline uint8_t  get_lc_flco()    { return d_lc_valid ? (d_lc[0] & 0x3f) : 0; };
	inline uint8_t  get_lc_fid()     { return d_lc_valid ? d_lc[1] : 0; };
	inline uint8_t  get_lc_svcopt()  { return d_lc_valid ? d_lc[2] : 0; };
	inline uint32_t get_lc_dstaddr() { return d_lc_valid ? ((d_lc[3] << 16) + (d_lc[4] << 8) + d_lc[5]) : 0; };
	inline uint32_t get_lc_srcaddr() { return d_lc_valid ? ((d_lc[6] << 16) + (d_lc[7] << 8) + d_lc[8]) : 0; };

	inline uint8_t  get_rc()         { return d_rc_valid ? d_rc : 0; };
	inline uint8_t  get_sb()         { return d_sb_valid ? d_sb : 0; };

	inline uint8_t  get_slot_cc()    { return (d_slot_type[0] << 3) + (d_slot_type[1] << 2) + (d_slot_type[2] << 1) + d_slot_type[3]; };
	inline uint8_t  get_data_type()  { return (d_slot_type[4] << 3) + (d_slot_type[5] << 2) + (d_slot_type[6] << 1) + d_slot_type[7]; };

};

#endif /* INCLUDED_DMR_SLOT_H */