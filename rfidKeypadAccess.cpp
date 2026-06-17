#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>

#define SS_PIN 10
#define RST_PIN 9

#define LED_VERDE 6
#define LED_VERMELHO 5
#define BUZZER 4

MFRC522 rfid(SS_PIN, RST_PIN);

// Teclado 4x3
const byte LINHAS = 4;
const byte COLUNAS = 3;

char teclas[LINHAS][COLUNAS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte pinosLinhas[LINHAS] = {2,3,7,8};
byte pinosColunas[COLUNAS] = {A0,A1,A2};

Keypad teclado = Keypad(
  makeKeymap(teclas),
  pinosLinhas,
  pinosColunas,
  LINHAS,
  COLUNAS
);

String senhaCorreta = "1234";
String senhaDigitada = "";

String uidAtual = "";
bool aguardandoSenha = false;

void setup() {

  Serial.begin(9600);

  SPI.begin();
  rfid.PCD_Init();

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  Serial.println("Sistema iniciado");
}

void loop() {

  if (!aguardandoSenha) {

    if (
      rfid.PICC_IsNewCardPresent() &&
      rfid.PICC_ReadCardSerial()
    ) {

      uidAtual = "";

      for (byte i = 0; i < rfid.uid.size; i++) {

        if (rfid.uid.uidByte[i] < 0x10)
          uidAtual += "0";

        uidAtual += String(
          rfid.uid.uidByte[i],
          HEX
        );
      }

      uidAtual.toUpperCase();

      senhaDigitada = "";
      aguardandoSenha = true;

      Serial.print("Cartao: ");
      Serial.println(uidAtual);

      Serial.println("Digite a senha");

      tone(BUZZER, 1500, 200);

      rfid.PICC_HaltA();
    }
  }

  else {

    char tecla = teclado.getKey();

    if (tecla) {

      tone(BUZZER, 2000, 100);

      if (tecla >= '0' && tecla <= '9') {

        senhaDigitada += tecla;

        Serial.print("*");
      }

      if (tecla == '*') {

        senhaDigitada = "";

        Serial.println();
        Serial.println("Senha limpa");
      }

      if (tecla == '#') {

        verificarSenha();
      }
    }
  }
}

void verificarSenha() {

  Serial.println();

  if (senhaDigitada != senhaCorreta) {

    Serial.println("Senha incorreta");

    digitalWrite(LED_VERMELHO, HIGH);

    tone(BUZZER, 500, 1000);

    delay(2000);

    digitalWrite(LED_VERMELHO, LOW);

    aguardandoSenha = false;

    return;
  }

  Serial.print("UID:");
  Serial.println(uidAtual);

  unsigned long inicio = millis();

  while (millis() - inicio < 5000) {

    if (Serial.available()) {

      String resposta =
        Serial.readStringUntil('\n');

      resposta.trim();

      if (resposta == "OK") {

        Serial.println("AUTORIZADO");

        digitalWrite(LED_VERDE, HIGH);

        tone(BUZZER, 2500, 300);

        delay(2000);

        digitalWrite(LED_VERDE, LOW);
      }

      else {

        Serial.println("NEGADO");

        digitalWrite(LED_VERMELHO, HIGH);

        tone(BUZZER, 500, 1000);

        delay(2000);

        digitalWrite(LED_VERMELHO, LOW);
      }

      break;
    }
  }

  aguardandoSenha = false;
}
