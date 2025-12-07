#include <SPI.h>
#include <LoRa.h>


#define LORA_PIN_SCK      18     // Clock SPI do LoRa
#define LORA_PIN_MISO     19    // MISO (Master In Slave Out)
#define LORA_PIN_MOSI     23    // MOSI (Master Out Slave In)
#define LORA_PIN_NSS      5    // Chip Select (às vezes chamado CS ou SS)
#define LORA_PIN_RESET    14    // Pino de reset do módulo LoRa
#define LORA_PIN_IRQ      26    // Pino de interrupção DIO0 (recepção de pacotes)

//Potência de saída
#define PA_OUTPUT_RFO        0
#define PA_OUTPUT_PA_BOOST   1


enum Protocol : uint8_t {
    STATUS_PROTOCOL = 0,
};

enum Operation : uint8_t {
    INITCOMM = 0,
};

struct SatPacket{
    uint8_t length;
    Protocol protocol;
    Operation operation;
};

SatPacket packet;

void sendPacket(Protocol protocol, Operation operation) {

    packet.length = sizeof(SatPacket);
    packet.protocol = protocol;
    packet.operation = operation;
    
    Serial.print("}-->\n\n");

    Serial.print("PACKAGE GENERATED: \n\n");

    Serial.print("SIZE: ");
    Serial.println(packet.length);

    Serial.print("PROTOCOL: ");
    Serial.println(packet.protocol);

    Serial.print("OPERATION: ");
    Serial.println(packet.operation);

    uint8_t *data = (uint8_t*)&packet;

    Serial.print("Sending Packet: ");
    Serial.print("{");
    for (int i = 0; i < packet.length; i++) {
        Serial.print((int)data[i]);
        Serial.print(" ");
    }
    Serial.println("}");
    

    LoRa.beginPacket();
    LoRa.write(data, packet.length);
    LoRa.endPacket();

    Serial.println("Package sent!\n");
    Serial.print("}-->\n\n");

    return;
}

void receivePacket() {
    int packetSize = LoRa.parsePacket();
    if (packetSize == 0) return;   // Nenhum pacote chegou ainda

    // Lê bytes do LoRa direto na struct global "packet"
    LoRa.readBytes((uint8_t*)&packet, sizeof(SatPacket));

    Serial.println("\n<--{ PACKET RECEIVED }");
    Serial.print("SIZE: "); Serial.println(packet.length);
    Serial.print("PROTOCOL: "); Serial.println(packet.protocol);
    Serial.print("OPERATION: "); Serial.println(packet.operation);

    Serial.print("Packet Bytes: { ");
    uint8_t *ptr = (uint8_t*)&packet;
    for (int i = 0; i < sizeof(SatPacket); i++) {
        Serial.print((int)ptr[i]);
        Serial.print(" ");
    }
    Serial.println("}");

    // Chama o processamento do protocolo
    onReceive();
}

void onReceive() {
    Serial.println("\nProcessing received packet...");

    switch (packet.protocol) {
            case STATUS_PROTOCOL:
            Serial.println("Protocol: STATUS_PROTOCOL");
            switch (packet.operation) {
                case INITCOMM:
                    Serial.println("Operation: INITCOMM");
                    Serial.println("GroundStation -> HI TRAINEESAT!!");
                    sendPacket(STATUS_PROTOCOL, INITCOMM);
                    Serial.println("Pacote de resposta enviado para a ground station.");
                    delay(1000);
                    break;
                default:
                    Serial.println("Operação desconhecida no protocolo: STATUS_PROTOCOL");
                    break;
            }
            break;
        default:
            Serial.println("Protocolo desconhecido!");
            break;
    }
    Serial.println("Packet processed.\n");
}


void setup() {
  Serial.begin(115200);
  while (!Serial);  
  delay(1000);

  Serial.println("\nWelcome to the Ecovision embedded software\n");

  Serial.println("Testing the Ecovision communication system with the LoRa Ra-02 RF module\n");
  
  Serial.println("Inicializando módulo LoRa...");

  SPI.begin(LORA_PIN_SCK, LORA_PIN_MISO, LORA_PIN_MOSI, LORA_PIN_NSS);

  LoRa.setPins(LORA_PIN_NSS, LORA_PIN_RESET, LORA_PIN_IRQ);

  if (!LoRa.begin(433E6)) {
    Serial.println("Falha na inicialização do LoRa.");
  }else{
    Serial.println("LoRa inicializado com sucesso.");
  }
  
  // Configura parâmetros
  LoRa.setSpreadingFactor(12);     // SF12
  LoRa.setSignalBandwidth(62500);  // 62.5 kHz
  LoRa.setCodingRate4(8);          // CR4/8
  LoRa.enableCrc();                // Habilita CRC
  LoRa.setPreambleLength(6);       // Preamble = 6

  // Define potência de transmissão máxima com PA_BOOST
  LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST);

  Serial.println("Entrando no loop de recepção de pacotes.\n");

}

void loop() {
  receivePacket();
}