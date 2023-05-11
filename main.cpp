//Display TFT-  ILI9341 Toutch

//Biblioteca
#include "mbed.h"
#include "Arduino.h"
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include "TouchScreen_kbv_mbed.h"
#include "Motor.h"
#include "main.h"

// Configuração Motor Y
DigitalIn confirma(PB_2);
AnalogIn EixoYJoyStick(PC_3);
InterruptIn emergencia(PB_11);
InterruptIn fdc2_y_(PB_1);

// variaveis auxiliares
bool REF = 0;
int joy_y;
int pos_y;

int cursor=1;
int cursor_pos=0;

bool flag_emergencia = 1;

Timer display;
Timer debounce;

struct pos {
  int y;
  int vol;
};

struct pega {
  int y;
};


pos posicao_1;
pos posicao_2;
pos posicao_3;
pos posicao_4;
pos posicao_5;
pos posicao_6;
pos posicao_7;
pos posicao_8;
pos posicao_9;

pega pos_pega;

pos posicoes[9] = {posicao_1,posicao_2,posicao_3,posicao_4,posicao_5,posicao_6,posicao_7,posicao_8,posicao_9};

// Configuração do Display
const PinName XP = D8, YP = A3, XM = A2, YM = D9; 
const int TS_LEFT=121,TS_RT=922,TS_TOP=82,TS_BOT=890;
DigitalInOut YPout(YP);
DigitalInOut XMout(XM);
TouchScreen_kbv ts = TouchScreen_kbv(XP, YP, XM, YM);
TSPoint_kbv tp;

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Orientação  Display
uint8_t Orientation = 1;

// Tabela de Cores

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// Serial
//Serial pc(USBTX, USBRX);

//***********************Escrita no  Display**********************************//
void print_posicao(){

    tft.fillScreen(BLACK);
    tft.setCursor(0, 0); // Orientação X,Y
    tft.print(posicoes[2].y);
    tft.println(" passos");
    tft.print(pos_y*3/200);
    tft.print(" mm");
}

void inicio_tela(){

    tft.fillScreen(BLACK);
    tft.setTextColor(CYAN);
    tft.setCursor(10, 10);
    tft.print("Referenciar");
    tft.setCursor(10, 50);
    if(REF==0){tft.setTextColor(RED);}
    tft.print("Pegar posicao");
    tft.setCursor(10, 90);
    tft.print("Pipetar");

    switch (cursor){
        case 1:
            tft.drawRoundRect(5,5,250,30,1,WHITE);
            break;
        case 2:
            tft.drawRoundRect(5,45,250,30,1,WHITE);
            break;
        case 3:
            tft.drawRoundRect(5,85,250,30,1,WHITE);
            break;
    }
}

void referenciamento_tela(){

    tft.fillScreen(BLACK);
    tft.setTextColor(CYAN);
    tft.setCursor(10, 85); // Orientação X,Y
    tft.printf("\rPor favor\naperte confirma\npara referenciar");
}

void emergencia_tela(){

    tft.fillScreen(BLACK);
    tft.setCursor(80, 10); // Orientação X,Y
    tft.setTextColor(RED);
    tft.print("\rEMERGENCIA\n");
    tft.setTextColor(CYAN);
    tft.println("\n\nDesative o botao\nquando for seguro");
}

void lista_pos_tela(){

    tft.fillScreen(BLACK);
    tft.setTextColor(RED);
    tft.setCursor(10, 10);
    tft.setTextSize(2);
    if(pos_pega.y>0){tft.setTextColor(CYAN);}
    tft.println("Posicao pega");
    tft.setTextColor(RED);
    tft.setCursor(10, 30);
    tft.println("Posicao 1");
    tft.setCursor(10, 50);
    tft.println("Posicao 2");
    tft.setCursor(10, 70);
    if(posicoes[2].y>0){tft.setTextColor(CYAN);}
    tft.println("Posicao 3");
    tft.setTextColor(RED);
    tft.setCursor(10, 90);
    tft.println("Posicao 4");
    tft.setCursor(10, 110);
    tft.println("Posicao 5");
    tft.setCursor(10, 130);
    tft.println("Posicao 6");
    tft.setCursor(10, 150);
    tft.println("Posicao 7");
    tft.setCursor(10, 170);
    tft.println("Posicao 8");
    tft.setCursor(10, 190);
    tft.println("Posicao 9");
    tft.setTextSize(3);
    tft.setTextColor(CYAN);
    tft.drawRoundRect(5,5+20*cursor_pos,160,24,1,WHITE);

    /*
    switch (cursor_pos){
        case 0:
            tft.drawRoundRect(5,5,160,20,1,WHITE);
            break;
        case 1:
            tft.drawRoundRect(5,5+20*cursor_pos,160,24,1,WHITE);
            break;
        case 2:
            tft.drawRoundRect(5,5+20*cursor_pos,160,25,1,WHITE);
            break;
        case 3:
            tft.drawRoundRect(5,5+20*cursor_pos,160,25,1,WHITE);
            break;
        case 4:
            tft.drawRoundRect(5,5+20*cursor_pos,160,25,1,WHITE);
            break;
        case 5:
            tft.drawRoundRect(5,5+20*cursor_pos,160,24,1,WHITE);
            break;
        case 6:
            tft.drawRoundRect(5,5+20*cursor_pos,160,25,1,WHITE);
            break;
        case 7:
            tft.drawRoundRect(5,5+20*cursor_pos,160,25,1,WHITE);
            break;
        case 8:
            tft.drawRoundRect(5,5+20*cursor_pos,160,25,1,WHITE);
            break;
        case 9:
            tft.drawRoundRect(5,5+20*cursor_pos,160,25,1,WHITE);
            break;
    }*/
}

//****************************************************************************//


// PROGRAMA PRINCIPAL ____________________________________________________________

void setup(void)
{
    //configs da tela
    tft.reset();
    tft.begin();
    tft.setRotation(Orientation);
    tft.fillScreen(BLACK);  // Fundo do Display
    tft.setTextColor(CYAN);
    tft.setTextSize(3);
    delay(1000);

    //inicia o debouncing
    debounce.start();

    //interrupções
    emergencia.fall(&desastre);

    
    inicio_tela();
    
}

void loop(){

    flag_emergencia = 1;

    joy_y = EixoYJoyStick.read() * 1000;

    if (joy_y<400){
        cursor++;
        if (cursor>3){
            cursor = 1;
        }
        inicio_tela();

        delay(300);

    } else if (joy_y>600){
        cursor--;
        if(cursor<1){
            cursor=3;
        }

        inicio_tela();
        delay(300);

    }

    if (confirma==1){
        switch(cursor){
            case 1:
                referenciamento_tela();
                wait(0.5);
                while(1){
                    if(confirma){
                        estado_ref();
                        delay(300);
                        inicio_tela();
                        break;
                    }
                }
                break;

//______________JOG_________________________________________________________________________________
            case 2:
                lista_pos_tela();
                delay(300);

                while(flag_emergencia){

                    joy_y = EixoYJoyStick.read() * 1000;

                    if (joy_y<400){
                        cursor_pos++;
                        if (cursor_pos>9){
                            cursor_pos = 0;
                            }
                        lista_pos_tela();

                        delay(300);

                    } else if (joy_y>600){
                        cursor_pos--;
                        if(cursor_pos<0){
                            cursor_pos=9;
                        }

                        lista_pos_tela();
                        delay(300);

                    }

                    if (confirma){
                        delay(300);
                        //JOG
                        while (REF == 1){

                            joy_y = EixoYJoyStick.read() * 1000;

                            // a cada 2 segundos, printa a posição no display:
                            if (display.read_ms()>2000){ 
                                print_posicao();
                                display.reset();
                            }

                            // gira o motor de acordo com a leitura do Joystick
                            if (joy_y>600){
                        
                                pos_y = pos_y + gira_y_mais();

                            } else if (joy_y<400){

                                pos_y = pos_y + gira_y_menos();

                            } else {
                                stop_y();
                            }

                            if(confirma){
                                delay(300);
                                switch(cursor_pos){
                                    case 0:
                                        pos_pega.y = pos_y;
                                        break;
                                    default:
                                        posicoes[cursor_pos-1].y=pos_y;
                                        break;
                                    

                                }
                            lista_pos_tela();
                            break;
                            }
                        }
                    
                    }
                }
                break;
        }
    }

}



//***********************Funções gerais**********************************//

void desastre(){
    
        stop_y(); //para o motor
        REF = 0; //
        flag_emergencia = 0;
        emergencia_tela();
        while (emergencia == 0){ //enquanto etiver apertado

        }

        inicio_tela();

}

void estado_ref(){
    if (debounce.read_ms() >30 && REF==0 && emergencia==1){
        referencia();

        if (fdc2_y_==1){REF = 1;}
        
        pos_y = 0;
        display.start();
        inicio_tela();
    }

    debounce.reset();
}

void referencia(){


    while(fdc2_y_==0 && flag_emergencia==1){ //roda até bater no fim de curso 2
        
        gira_y_menos();

        if(emergencia==0){
            break;
        }

    }
    stop_y();
    
}