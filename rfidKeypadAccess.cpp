#include <SPI.h> // SPI que o RFID usa
#include <MFRC522.h> //lib do RFID
#include <Keypad.h> // lib teclado 4x3

//pinos RFID
#define SS_PIN 10
#define RST_PIN 9

//pinos leds e buzzer
#define LED_VERDE 6
#define LED_VERMELHO 5
#define BUZZER 4

// cria objeto RFID
MFRC522 rfid(SS_PIN, RST_PIN);

// Teclado 4x3
const byte LINHAS = 4;
const byte COLUNAS = 3;

//mapeamento das teclas
char teclas[LINHAS][COLUNAS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

//pinos das linhas e colunas
byte pinosLinhas[LINHAS] = {2,3,7,8};
byte pinosColunas[COLUNAS] = {A0,A1,A2};

//criação do objeto teclado
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
bool modoVisitante = false;

//inicializa comunicação com rasp, barramento SPI e leitor RFID. Além de configurar leds e buzzer
void setup() {

  Serial.begin(9600);

  SPI.begin();
  rfid.PCD_Init();

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  Serial.println("Sistema iniciado");
}

//loop principal
void loop() {

  //leitura da tecla pressionada
  char tecla = teclado.getKey();
  
  //mostra tecla pressionada no monito
  if (tecla) {
    Serial.print("Tecla=");
    Serial.println(tecla);
}

  // som ao clicar
  if (tecla) {

    tone(BUZZER, 1800, 40);

    //ativa modo visitante ao clicar "*" para assim colocar a senha(opção caso não tenha TAG) 
  if (tecla == '*' && !aguardandoSenha) {

    modoVisitante = true;
    aguardandoSenha = true;

    Serial.println("Modo visitante ativado");
  }

  // modo aguardando senha também é ativado ao clique do "*"  
  if (aguardandoSenha) {

    if (tecla >= '0' && tecla <= '9') {

      senhaDigitada += tecla;

      Serial.print("Senha atual: ");
      Serial.println(senhaDigitada);
    }
    // clicar # confirma a senha
    if (tecla == '#') {

      verificarSenha();
    }
  }
}
  //caso o sistema não esteja aguardando senha entra nessa condição
  //que é usar RFID (TAG)
  if (!aguardandoSenha) {

    //verifica se é uma nova TAG
    if (
      rfid.PICC_IsNewCardPresent() &&
      rfid.PICC_ReadCardSerial()
    ) {

      uidAtual = "";

      //estrutura o UID da TAG
      for (byte i = 0; i < rfid.uid.size; i++) {

        if (rfid.uid.uidByte[i] < 0x10)
          uidAtual += "0";

        uidAtual += String(
          rfid.uid.uidByte[i],
          HEX
        );
      }

uidAtual.toUpperCase();

//som da leitura da TAG
tone(BUZZER, 1500, 200);

Serial.print("UID:");
Serial.println(uidAtual);

//comunicação com a rasp para verificar o UID e condições em caso de autorizado ou negado
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
// finaliza comunicação com a TAG
rfid.PICC_HaltA();
    }
  }}

// som tocado ao ter o acesso com o modo visitante  
void somVisitante() {

  tone(BUZZER, 523, 100); // Dó
  delay(120);

  tone(BUZZER, 659, 100); // Mi
  delay(120);

  tone(BUZZER, 784, 100); // Sol
  delay(120);

  tone(BUZZER, 1047, 300); // Dó agudo
  delay(350);

  noTone(BUZZER);
}

//método chamado sempre que é necessário verificar a senha
void verificarSenha() {

  Serial.println();

  //em caso de senha incorreta
  if (senhaDigitada != senhaCorreta) {
  
    Serial.println("Senha incorreta");
  
    // envia para a Raspberry registrar tentativa
    Serial.println("SENHA_ERRADA");
    
    //sinalização de buzzer e led
    digitalWrite(LED_VERMELHO, HIGH);
  
    tone(BUZZER, 500, 1000);
  
    delay(2000);
  
    digitalWrite(LED_VERMELHO, LOW);
    
    //reseta dados
    aguardandoSenha = false;
    modoVisitante = false;
    senhaDigitada = "";

    Serial.print("Senha recebida: ");
    Serial.println(senhaDigitada);

    Serial.print("Senha correta: ");
    Serial.println(senhaCorreta);
  
    return;
  }

  //em caso de senha correta
  if (modoVisitante) {

    //comunica rasp que é visitante
    Serial.println("VISITANTE");
    somVisitante();

  } else {

    //envia UID para verificação em caso de entrada com TAG
    Serial.print("UID:");
    Serial.println(uidAtual);
  }

  //comunicação com a rasp para verificar o UID e condições em caso de autorizado ou negado novamente
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
  //reseta dados para usar novamente
  aguardandoSenha = false;
  modoVisitante = false;
}
