#include "api.h"

union converter {
  int32_t fullData; //occupies 4 bytes
  uint8_t bytes[4]; //occupies 4 bytes
};

void writeUart(uint8_t packet_id, int32_t value) {
  union converter out_data;
  out_data.fullData = value;

  fputc(0xFA, stdout);
  fputc(packet_id, stdout);
  uint8_t checksum = 255;

  for (int i = 0; i < 4; i++) {
    checksum -= out_data.bytes[i];
    fputc(out_data.bytes[i], stdout);
  }

  // printf("FA:%02x:%02x:%02x:%02x:%02x:%02x\n", packet_id,
  // out_data.bytes[0], out_data.bytes[1], out_data.bytes[2], out_data.bytes[3],
  // checksum);

  fputc(checksum, stdout);
}

void readUart(uint8_t *packet_id, int32_t *value) {
  union converter in_data;

  //TODO: This needs to be done using getchar() instead
  // if (fcount(stdin) < 7) {
  //   return;
  // }

  // printf("checking uart.... %d bytes available\n", fcount(stdin));

  uint8_t startByte = fgetc(stdin); // TODO -- check for partial data
  if (startByte != 0xFA) {
    // printf("wrong startByte  %02x", startByte);
    return;
  }

  *packet_id = fgetc(stdin);

  uint8_t checksum_calc = 255;
  for (int i = 0; i < 4; i++) {
    in_data.bytes[i] = fgetc(stdin);
    checksum_calc -= in_data.bytes[i];
  }

  uint8_t checksum_given = fgetc(stdin);

  //   printf("%02x:%02x:%02x:%02x:%02x:%02x:%02x\n", startByte, packet_id,
  //     in_data.bytes[0], in_data.bytes[1], in_data.bytes[2], in_data.bytes[3],
  //     checksum_given);

  if (false && checksum_calc == checksum_given) {
    *packet_id = 0;
  } else {
    *value = in_data.fullData;
  }
}
