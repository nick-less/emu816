
/**
 * emulate a SPI65/B CPLD
 *
 */
#include "spi.hpp"
#include "Log.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <fstream>
#include <iostream>
using namespace std;

#define TRACE_SD 1
#define TRACE_SPI 1

#define CMD_NONE 0x00
#define SPI_WRSR 0x01 // Write Status Register
#define SPI_WRITE 0x02 // Write data to memory array
#define SPI_READ 0x03 // Read data from memory
#define SPI_WRDI 0x04 // Reset Write Enable Latch
#define SPI_RDSR1 0x05 // Read Status Register 1
#define SPI_WREN 0x06 // Set Write Enable Latch
#define SPI_RDSR2 0x35 // Read Status Register 2
#define SPI_FAST_READ                                                          \
  0x0b // Similar to the READ command, but possibly uses a faster clock
#define SPI_SE 0xD8 // Erase one sector in memory
#define SPI_BE 0xC7 // Erase all memory
#define SPI_DP 0xb9 // Write Enable Command

#define SPI_RES 0xab //	Read Electonic Signature
#define SPI_RDID 0x9F // reads the ID of the SPI Flash

class SPIDummy : public SPIDevice {
public:
  virtual void select(void) {}
  virtual void unselect(void) {}
  virtual uint8_t read(void) { return 0x42; }
  virtual void write(uint8_t value) {}
};



class SDCard : public SPIDevice {
private:
  int trace =1 ;
  int sd_mode = 0;

  uint8_t outbyte;
  uint8_t sd_bits;
  uint8_t sd_bitct;
  uint8_t sd_miso;
  int sd_cmdp = 0;
  int sd_ext = 0;
  uint8_t sd_cmd[6];
  uint8_t sd_in[520];
  int sd_inlen, sd_inp;
  uint8_t sd_out[520];
  int sd_outlen, sd_outp;
  int sd_fd = -1;
  off_t sd_lba;
  const uint8_t sd_csd[17] = {

      0xFE, /* Sync byte before CSD */
      /* Taken from a Toshiba 64MB card c/o softgun */
      0x00, 0x2D, 0x00, 0x32, 0x13, 0x59, 0x83, 0xB1, 0xF6, 0xD9, 0xCF, 0x80,
      0x16, 0x40, 0x00, 0x00};

  uint8_t sd_process_command(void) {
    if (sd_ext) {
      sd_ext = 0;
      switch (sd_cmd[0]) {
      case 0x40 + 41:
        return 0x00;
      default:
        return 0xFF;
      }
    }
    if (trace & TRACE_SD)
      fprintf(stderr, "Command received %x\n", sd_cmd[0]);
    switch (sd_cmd[0]) {
    case 0x40 + 0: /* CMD 0 */
      return 0x01; /* Just respond 0x01 */
    case 0x40 + 1: /* CMD 1 - leave idle */
      return 0x00; /* Immediately indicate we did */
    case 0x40 + 9: /* CMD 9 - read the CSD */
      memcpy(sd_out, sd_csd, 17);
      sd_outlen = 17;
      sd_mode = 2;
      sd_outp = 0;
      return 0x00;
    case 0x40 + 16: /* CMD 16 - set block size */
      /* Should check data is 512 !! FIXME */
      return 0x00;  /* Sure */
    case 0x40 + 13: /* Status */
      sd_out[0] = 0;
      sd_outlen = 1;
      sd_outp = 0;
      sd_mode = 2;
      return 0x00;  /* Whatever */
    case 0x40 + 17: /* Read */
      sd_outlen = 514;
      sd_outp = 0;
      /* Sync mark then data */
      sd_out[0] = 0xFF;
      sd_out[1] = 0xFE;
      sd_lba = sd_cmd[4] + 256 * sd_cmd[3] + 65536 * sd_cmd[2] +
               16777216 * sd_cmd[1];
      if (trace & TRACE_SD)
        fprintf(stderr, "Read LBA %lx\n", (long)sd_lba);
      if (lseek(sd_fd, sd_lba, SEEK_SET) < 0 ||
          ::read(sd_fd, sd_out + 2, 512) != 512) {
        if (trace & TRACE_SD)
          fprintf(stderr, "Read LBA failed.\n");
        return 0x01;
      }
      sd_mode = 2;
      /* Result */
      return 0x00;
    case 0x40 + 24: /* Write */
      /* Will send us FE data FF FF */
      if (trace & TRACE_SD)
        fprintf(stderr, "Write LBA %lx\n", (long)sd_lba);
      sd_inlen = 514; /* Data FF FF */
      sd_lba = sd_cmd[4] + 256 * sd_cmd[3] + 65536 * sd_cmd[2] +
               16777216 * sd_cmd[1];
      sd_inp = 0;
      sd_mode = 4; /* Send a pad then go to mode 3 */
      return 0x00; /* The expected OK */
    case 0x40 + 55:
      sd_ext = 1;
      return 0x01;
    default:
      return 0x7F;
    }
  }

  uint8_t sd_process_data(void) {
    switch (sd_cmd[0]) {
    case 0x40 + 24: /* Write */
      sd_mode = 0;
      if (lseek(sd_fd, sd_lba, SEEK_SET) < 0 ||
          ::write(sd_fd, sd_in, 512) != 512) {
        if (trace & TRACE_SD)
          fprintf(stderr, "Write failed.\n");
        return 0x1E; /* Need to look up real values */
      }
      return 0x05; /* Indicate it worked */
    default:
      sd_mode = 0;
      return 0xFF;
    }
  }

  uint8_t sd_card_byte(uint8_t in) {
    /* No card present */
    if (sd_fd == -1)
      return 0xFF;

    if (sd_mode == 0) {
      if (in != 0xFF) {
        sd_mode = 1; /* Command wait */
        sd_cmdp = 1;
        sd_cmd[0] = in;
      }
      return 0xFF;
    }
    if (sd_mode == 1) {
      sd_cmd[sd_cmdp++] = in;
      if (sd_cmdp == 6) { /* Command complete */
        sd_cmdp = 0;
        sd_mode = 0;
        /* Reply with either a stuff byte (CMD12) or a
           status */
        return sd_process_command();
      }
      /* Keep talking */
      return 0xFF;
    }
    /* Writing out the response */
    if (sd_mode == 2) {
      if (sd_outp + 1 == sd_outlen)
        sd_mode = 0;
      return sd_out[sd_outp++];
    }
    /* Commands that need input blocks first */
    if (sd_mode == 3) {
      sd_in[sd_inp++] = in;
      if (sd_inp == sd_inlen)
        return sd_process_data();
      /* Keep sending */
      return 0xFF;
    }
    /* Sync up before data flow starts */
    if (sd_mode == 4) {
      /* Sync */
      if (in == 0xFE)
        sd_mode = 3;
      return 0xFF;
    }
    return 0xFF;
  }

  uint8_t spi_byte_sent(uint8_t val) {
    uint8_t r = sd_card_byte(val);
    if (trace & TRACE_SPI)
      fprintf(stderr, "[SPI %02X:%02X]\n", val, r);
    fflush(stdout);
    return r;
  }

  void spi_select(uint8_t val) {
    if (val) {
      if (trace & TRACE_SPI)
        fprintf(stderr, "[Raised \\CS]\n");
      sd_bits = 0;
      sd_bitct = 0;
      sd_mode = 0; /* FIXME: layering */
      return;
    } else {
      if (trace & TRACE_SPI)
        fprintf(stderr, "[Lowered \\CS]\n");
    }
  }

  void spi_clock(void) {
    // static uint8_t rxbits = 0xFF;

    // if (!qreg[0]) {
    //   fprintf(stderr, "SPI clock: no op.\n");
    //   return;
    // }

    // if (trace & TRACE_SPI)
    //   fprintf(stderr, "[SPI clock - txbit = %d ", qreg[5]);
    // sd_bits <<= 1;
    // sd_bits |= qreg[5];
    // sd_bitct++;
    // if (sd_bitct == 8) {
    //   rxbits = spi_byte_sent(sd_bits);
    //   sd_bitct = 0;
    // }
    // /* Falling edge */
    // sd_miso = (rxbits & 0x80) ? 0x01 : 0x00;
    // rxbits <<= 1;
    // rxbits |= 0x01;
    // if (trace & TRACE_SPI)
    //   fprintf(stderr, "rxbit = %d]\n", sd_miso);
  }
public:
  SDCard() {
  }
  ~SDCard() {}

  virtual void select(void) {
    spi_select(1);
  }
  virtual void unselect(void) {
    spi_select(0);
  }
  virtual uint8_t read(void) { 
    return outbyte; 
  }
  virtual void write(uint8_t value) {
      outbyte = spi_byte_sent(value);
  }
};


/**
 * emulate a spi attached flash
 */
class SPIFlash : public SPIDevice {
private:
  uint8_t cmd;
  uint32_t cnt;
  uint32_t startAdr;
  uint8_t *content;

public:
  SPIFlash() {
    cmd = CMD_NONE;
    ifstream is;
    is.open("../native/flashImage.bin", ios::in | ios::binary);
    if (is.is_open()) {
      is.seekg(0, ios::end);
      int size = is.tellg();
      content = new uint8_t[size];
      is.seekg(0, ios::beg);
      is.read((char *)content, size);
      is.close();
      Log::vrb("SPIDevice").str("read ").hex(size).sp().str("bytes").show();
    }
  }
  ~SPIFlash() {}

  virtual void select(void) {
    Log::vrb("SPIDevice").str("select ").show();
    cmd = CMD_NONE;
  }
  virtual void unselect(void) {
    Log::vrb("SPIDevice").str("unselect ").show();
    cmd = CMD_NONE;
  }

  virtual uint8_t read(void) {
    switch (cmd) {
    case SPI_READ:
      uint8_t v = content[startAdr++];
      Log::vrb("SPIDevice").str("read: ").hex(startAdr).str(": ").hex(v).show();
      return v;
    }
    return 0xee;
  }
  virtual void write(uint8_t value) {
    Log::vrb("SPIDevice").str("write: ").hex(value).show();

    if (cmd == CMD_NONE) {
      cmd = value;
      cnt = 0;
      startAdr = 0;
      // Log::vrb("SPIDevice").str("cmd READ: ").hex(startAdr).show();

      return;
    }
    if (cmd == SPI_READ) {
      uint32_t v = value;
      if (cnt == 0) {
        v = v << 16;
        startAdr = startAdr | v;
      }
      if (cnt == 1) {
        v = v << 8;
        startAdr = startAdr | v;
      }

      if (cnt == 2) {
        startAdr = startAdr | value;
      }
      cnt++;
    }
  }
};


SPI::SPI(Address adr) {
  startAdr = adr;
  devices[0] = new SPIDummy();
  devices[1] = new SPIDummy();
  devices[2] = new SPIFlash();
  devices[3] = new SPIDummy();
}

SPI::~SPI() {}
void SPI::setCpu(Cpu65816 *cpu) { this->cpu = cpu; }

void SPI::setDevice(int deviceId, SPIDevice *device) {
  devices[deviceId] = device;
}

void SPI::storeByte(const Address &address, uint8_t value) {

  /*  Log::vrb("SPI")
        .str("set: ")
        .hex(address.getOffset())
        .str(": ")
        .hex(value)
        .show();
  */
  switch (address.getOffset() - startAdr.getOffset()) {
  case 0:
    switch (spi_select & 0xf) {
    case 1:
      devices[0]->write(value);
      break;
    case 2:
      devices[1]->write(value);
      break;
    case 4:
      devices[2]->write(value);
      break;
    case 8:
      devices[3]->write(value);
      break;
    }
    break;
  case 1:
    spi_status = value;
    break;
  case 2:
    spi_divisor = value;
    break;
  case 3:
    for (int i = 0; i < 4; i++) {
      if ((spi_select & 0x01) && !(value & 0x01)) {
        devices[0]->unselect();
      }
    }
    spi_select = value;
    switch (spi_select & 0xf) {
    case 1:
      devices[0]->select();
      break;
    case 2:
      devices[1]->select();
      break;
    case 4:
      devices[2]->select();
      break;
    case 8:
      devices[3]->select();
      break;
    }
    break;
  }
}

uint8_t SPI::readByte(const Address &address) {
  // Log::vrb("SPI").str("read: ").hex(address.getOffset()).show();
  switch (address.getOffset() - startAdr.getOffset()) {
  case 0:
    switch (spi_select & 0xf) {
    case 1:
      return devices[0]->read();
    case 2:
      return devices[1]->read();
    case 4:
      return devices[2]->read();
    case 8:
      return devices[3]->read();
    }

    return 0;
  case 1:
    return 0x80;

  case 2:
  case 3:
    return 0;
  }

  return 0xEA;
}

bool SPI::decodeAddress(const Address &in, Address &out) {

  if ((in.getBank() == startAdr.getBank()) &&
      (in.getOffset() >= startAdr.getOffset()) &&
      (in.getOffset() < startAdr.getOffset() + size)) {
    out = in;
    return true;
  }

  return false;
}
