#include "api.h"
extern int32_t inp_buffer_available();
extern int bytes_in_buffer;


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
    data[13]
    );
    bytes_in_buffer -=14;

}
void readUart(uint8_t *packet_id, int32_t *value,int line) {
  union converter in_data;

  uint8_t startByte = 0;
  uint8_t data[7];
  bytes_in_buffer += fcount(stdin);

  fread(data,1,1,stdin);
  bytes_in_buffer -=1;
  startByte = data[0];

  if(startByte != 0xFA){
    *packet_id = 0;//just to make sure its denied
    return;
  }
  bytes_in_buffer += fcount(stdin);
  fread(&data[1],1,6,stdin);
  *packet_id = data[1];
  bytes_in_buffer -=6;

  uint8_t checksum_calc = 255;
  for (int i = 0; i < 4; i++) {
    in_data.bytes[i] = data[i+2];
    checksum_calc -= data[i+2];
  }

  uint8_t checksum_given = data[6];

  display_center_printf(line,"%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
    data[0],
    data[1],
    data[2],
    data[3],
    data[4],
    data[5],
    data[6]
    );


  if (false && checksum_calc == checksum_given) {
    *packet_id = 0;
  } else {
    *value = in_data.fullData;
  }
}
