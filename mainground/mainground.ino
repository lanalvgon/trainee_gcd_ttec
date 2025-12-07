#include <SPI.h>
#include <LoRa.h>


#define LORA_PIN_SCK      18     // Clock SPI do LoRa
#define LORA_PIN_MISO     19    // MISO (Master In Slave Out)
#define LORA_PIN_MOSI     23    // MOSI (Master Out Slave In)
#define LORA_PIN_NSS      5    // Chip Select (às vezes chamado CS ou SS)
#define LORA_PIN_RESET    14    // Pino de reset do módulo LoRa
#define LORA_PIN_IRQ      26    // Pino de interrupção DIO0 (recepção de pacotes)

//Define Potência de saída
#define PA_OUTPUT_RFO        0
#define PA_OUTPUT_PA_BOOST   1

// associa o protocolo de status ao número 0
enum Protocol : uint8_t {
    STATUS_PROTOCOL = 0,
};

// associa a operação initicom ao número 0
enum Operation : uint8_t {
    INITCOMM = 0,
};

// cria uma estrutura do tipo satpacket

struct SatPacket{
    uint8_t length;
    Protocol protocol;
    Operation operation;
};

// cria um pacote do tipo satpacket
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

bool receivePacket(SatPacket *packet, unsigned long timeout_ms) {
    unsigned long start = millis();

    while (millis() - start < timeout_ms) {
        int packetSize = LoRa.parsePacket();
        if (packetSize == 0) continue;  // Nenhum pacote ainda

        Serial.println("\n<--{ PACKET RECEIVED }");

        if (packetSize != sizeof(SatPacket)) {
            Serial.println("Packet size mismatch!");
            Serial.print("Expected: "); Serial.println(sizeof(SatPacket));
            Serial.print("Received: "); Serial.println(packetSize);
            while (LoRa.available()) LoRa.read(); // limpa buffer
            return false;
        }

        uint8_t buffer[sizeof(SatPacket)];
        int index = 0;
        
        while (LoRa.available() && index < sizeof(SatPacket)) {
            buffer[index++] = (uint8_t)LoRa.read();
        }

        memcpy(packet, buffer, sizeof(SatPacket));

        Serial.println("Decoded Packet Content:");
        Serial.print("SIZE: "); Serial.println(packet->length);
        Serial.print("PROTOCOL: "); Serial.println(packet->protocol);
        Serial.print("OPERATION: "); Serial.println(packet->operation);

        Serial.print("Packet Bytes: { ");
        for (int i = 0; i < sizeof(SatPacket); i++) {
            Serial.print((int)buffer[i]);
            Serial.print(" ");
        }
        Serial.println("}");
        Serial.println("<--}\n");

        return true;
    }

    Serial.println("Timeout: no response received.\n");
    return false;
}


void onReceive() {
    Serial.println("\nProcessing received packet...");

    switch (packet.protocol) {
            case STATUS_PROTOCOL:
            Serial.println("Protocol: STATUS_PROTOCOL");
            switch (packet.operation) {
                case INITCOMM:
                    Serial.println("Ecovision -> HI GROUND!!");
                    break;
                default:
                    Serial.println("Operação desconhecida no protocolo STATUS_PROTOCOL");
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

  Serial.println("\nWelcome to the Ecovision ground station embedded software\n");

  Serial.println("Testing the Ecovision Ground Station communication system with the LoRa Ra-02 RF module\n");
  
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

}

void loop() {
  String comand = "";

  while (!Serial.available()) {
    delay(10);
  }

  comand = Serial.readStringUntil('\n');
  comand.trim(); 


  if (comand == "initcomm") {
    sendPacket(STATUS_PROTOCOL, INITCOMM); //  envia o pacote referente à "oi" 
    if (receivePacket(&packet, 5000)) {  // tempo de 5 segundos
        Serial.println("Resposta Recebida!");
        onReceive();
    }
  }else {
    Serial.println("Error: nonexistent command.");
  }
}