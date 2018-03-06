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

uint8_t incomplete_data[255];
size_t  buffer_len = 0;

void readUart(void (*callback)(uint8_t packet_id, int32_t value)) {
  int read = fread(incomplete_data + buffer_len, sizeof(uint8_t),
    255 - buffer_len, stdin);

  if (read > 0) {
    buffer_len += read;
  } else {
    printf("error: no bytes read from stdin\n");
    return;
  }
  printf("buffer_len: %d\n", buffer_len);

  int consumed = 0;
  for (int offset = 0; offset < buffer_len; ++offset) {
    if (incomplete_data[offset] == 0x5f && buffer_len - offset > 7) {
      union converter in_data;
      uint8_t packet_id = incomplete_data[offset+1];
      uint8_t checksum_calc = 255;
      for (int i = 0; i < 4; i++) {
        in_data.bytes[i] = incomplete_data[offset+2+i];
        checksum_calc -= in_data.bytes[i];
      }
      uint8_t checksum_given = incomplete_data[offset+6];

      offset += 6;
      consumed = offset;

      if (checksum_calc == checksum_given) {
        printf("accepted packet {%d : %ld} \n", packet_id, in_data.fullData);
        callback(packet_id, in_data.fullData);
      }
    }
  }

  printf("consumed %d bytes\n", consumed);
  for (int i = 0; i < buffer_len - consumed; ++i) {
    incomplete_data[i] = incomplete_data[i + consumed];
  }
  buffer_len -= consumed;
}
