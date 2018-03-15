#include "api.h"
extern int32_t inp_buffer_available();


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
int fcount(FILE* file){
  return inp_buffer_available();
}
extern int read_bytes;
void testUart(){
  uint8_t data[14];
  fread(data,1,14,stdin);
  display_center_printf(9,"%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
    data[0],
    data[1],
    data[2],
    data[3],
    data[4],
    data[5],
    data[6]
    );
  display_center_printf(10,"%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
    data[7],
    data[8],
    data[9],
    data[10],
    data[11],
    data[12],
    data[13],
    data[14]
    );
}
void readUart(uint8_t *packet_id, int32_t *value) {
  union converter in_data;

  uint8_t startByte = 0;
  /*while (fcount(stdin) > 6){
    startByte = fgetc(stdin);
    read_bytes++;
    if(startByte == 0xFA){
      break;
    }
  }*/
  startByte = fgetc(stdin);
  
  if(startByte != 0xFA){
    *packet_id = 0;//just to make sure its denied
    return;
  }

  // printf("checking uart.... %d bytes available\n", fcount(stdin));

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
