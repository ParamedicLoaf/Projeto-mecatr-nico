//Display TFT-  ILI9341 Toutch

//Biblioteca
#include "mbed.h"
#include "Arduino.h"
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include "TouchScreen_kbv_mbed.h"
#include "Motor.h"
#include "main.h"

//IHM
DigitalIn confirma(PB_2);
DigitalIn voltar(PA_12);
AnalogIn EixoYJoyStick(PC_3);
AnalogIn EixoXJoyStick(PC_2);
InterruptIn emergencia(PB_11);

// pipeta
DigitalOut pipeta(PC_11);

//Motor Y
InterruptIn fdc2_y_(PB_1);

//Motor X
InterruptIn fdc2_x_(PB_1);

// variaveis auxiliares
bool REF = 0;
bool flag_emergencia = 1;
bool PIP = 0;
bool pipeta_cheia = 0;

int joy_y;
int pos_y;
int joy_x;
int pos_x;

int cursor=1;
int cursor_pos=0;


Timer display;
Timer debounce;

struct pos {
  int y;
  int x;
  int vol;
};

struct pega {
  int y;
  int x;
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

int volume;

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


//***********************Escrita no  Display**********************************//

void limpa_tela(){
    tft.fillScreen(BLACK);
}

void pipetando_tela(pos posicao, int i){
    tft.setCursor(10, 0); // Orientação X,Y
    tft.printf("Posicao %d",i);
    tft.setCursor(10, 30);
    tft.fillRect(50,30,250,30,BLACK);
    tft.printf("Faltam %d ml",posicao.vol);
}

void pipetagem_concluida_tela(){
    tft.fillScreen(BLACK);
    tft.setCursor(50, 200); // Orientação X,Y
    tft.setTextSize(2);
    tft.printf("PIPETAGEM CONCLUIDA");
    tft.setTextSize(3);
}

void print_posicao(){

    tft.fillScreen(BLACK);
    tft.setCursor(0, 0); // Orientação X,Y
    tft.print(pos_y);
    tft.println(" passos");
    tft.print(pos_y*3/200);
    tft.print(" mm");
}

void pega_volume_tela(){

    tft.fillScreen(BLACK);
    tft.setCursor(10, 10); // Orientação X,Y
    tft.printf("y = %d mm",pos_y*3/200);
    tft.setCursor(10, 40); // Orientação X,Y
    tft.printf("x = %d mm",pos_x*3/200);
    tft.setCursor(10, 70);
    tft.printf("%d ml",volume);
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
    if(PIP==0){tft.setTextColor(RED);}
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
    
    
    for (int i=1;i<10;i++){
        
        if(posicoes[i-1].vol>0){
            tft.setTextColor(CYAN);
            tft.setCursor(150, 10+i*20);
            tft.printf("%d ml", posicoes[i-1].vol);
        }
        tft.setCursor(10, 10+i*20);
        tft.printf("Posicao %d:",i);
        tft.setTextColor(RED);
    }
    
    tft.setTextSize(3);
    tft.setTextColor(CYAN);
    tft.drawRoundRect(5,5+20*cursor_pos,210,24,1,WHITE);

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

    pipeta = 1;

    //inicia o debouncing
    debounce.start();
    display.start();

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

    if (!confirma){ // função selecionada
        switch(cursor){
            case 1:
                referenciamento_tela();
                wait(0.5);
                while(1){
                    if(!confirma){ //AAAAAAAAAAAAAAAAAAAA
                        estado_ref();
                        delay(300);
                        inicio_tela();
                        break;
                    }
                    if(!voltar){
                        delay(300);
                        inicio_tela();
                        break;
                    }
                }
                break;

//______________JOG_________________________________________________________________________________
            case 2:
                if(REF){
                    seleciona_posicao();
                }
                
                break;

//________PIPETAGEM________________________________________________________________________________
            case 3:
                if(PIP && REF){
                    pipetagem();
                }
                break;
        }
    }

}



//***********************Funções gerais**********************************//

void desastre(){
    
        stop_y(); //para o motor
        stop_x();
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

        if (fdc2_y_==1 && fdc2_x_==1){REF = 1;}
        
        pos_y = 0;
        pos_x = 0;
        
        inicio_tela();
    }

    debounce.reset();
}

void referencia(){


    while((fdc2_y_==0||fdc2_x_==0) && flag_emergencia==1){ //roda até bater no fim de curso 2
        
        gira_y_menos();
        gira_x_menos();

        if(emergencia==0){
            break;
        }

    }
    stop_y();
    
}

void seleciona_posicao(){
    lista_pos_tela();
    delay(300);

    while(flag_emergencia){

        if(!voltar){
            inicio_tela();
            delay(300);
            break;
        }

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

        if (!confirma){ //movimentando pos específica
            delay(300);
            //JOG
            while (REF == 1){

                if(!voltar){
                    lista_pos_tela();
                    delay(300);
                    break;
                }

                joy_y = EixoYJoyStick.read() * 1000;
                joy_x = EixoXJoyStick.read() * 1000;

                // gira o motor Y de acordo com a leitura do Joystick
                if (joy_y>600){
            
                    pos_y = pos_y + gira_y_menos();

                } else if (joy_y<400){

                    pos_y = pos_y + gira_y_mais();

                } else {
                    stop_y();
                    // a cada 2 segundos, printa a posição no display:
                    if (display.read_ms()>2000){ 
                        print_posicao();
                        display.reset();
                    }
                    
                }
                /*else {
                    stop_y();
                    // a cada 2 segundos, printa a posição no display:
                    if (display.read_ms()>2000){ 
                        print_posicao();
                        display.reset();
                    }
                }*/

                // gira o motor X de acordo com a leitura do Joystick
                if (joy_x>600){
            
                    pos_x = pos_x + gira_x_menos();

                } else if (joy_x<400){

                    pos_x = pos_x + gira_x_mais();

                } else {
                    stop_x();
                }

                if(!confirma){ //confirmar posições
                    delay(300);
                    switch(cursor_pos){
                        case 0:
                            pos_pega.y = pos_y;
                            pos_pega.x = pos_x;
                            break;
                        default:
                            posicoes[cursor_pos-1].y=pos_y;
                            posicoes[cursor_pos-1].x=pos_x;

                            pega_volume_tela();
                            while(flag_emergencia){

                                joy_y = EixoYJoyStick.read() * 1000;
                                
                                if (joy_y>600){
            
                                    volume++;
                                    pega_volume_tela();
                                    delay(300);

                                } else if (joy_y<400){

                                    volume--;
                                    if(volume<0){
                                        volume=0;
                                    }
                                    pega_volume_tela();
                                    delay(300);

                                }

                                if (!confirma){ 
                                    delay(300);
                                    posicoes[cursor_pos-1].vol=volume;
                                    if(pos_pega.y>0){PIP = 1;}
                                    break;
                                }
                                if(!voltar){
                                    lista_pos_tela();
                                    delay(300);
                                    break;
                                    }
                            }
                            
                            break;

                    }
                lista_pos_tela();
                if(flag_emergencia==0){inicio_tela();}
                break;
                }
            }
        }
    }
}

void pipetagem(){

    limpa_tela();
    //pipetando_tela(posicoes[0],1);

    for (int i=0;i<9;i++){

        limpa_tela();

        pipetando_tela(posicoes[i],i+1);

        while(posicoes[i].vol>0 && flag_emergencia){
            if (pipeta_cheia==0){

                while(REF && flag_emergencia){

                    // Movimenta y até a posição de pega
                    if(pos_y<pos_pega.y){
                        pos_y = pos_y +gira_y_mais();
                    } else if(pos_y>pos_pega.y) {
                        pos_y = pos_y +gira_y_menos();
                    }
                    
                    // Movimenta x até a posição de pega
                    if(pos_x<pos_pega.x){
                        pos_x = pos_x +gira_x_mais();
                    } else if(pos_x>pos_pega.x) {
                        pos_x = pos_x +gira_x_menos();
                    }

                    if(pos_x==pos_pega.x && pos_y==pos_pega.y){
                        pipeta = 0;
                        delay(300);
                        pipeta = 1;
                        pipeta_cheia = 1;
                        wait(1.6);
                        break;
                    }

                    if(emergencia==0){
                        break;
                    }

                }
            } else {

                while(REF && flag_emergencia){

                    // Movimenta y até a posição salva
                    if(pos_y<posicoes[i].y){
                        pos_y = pos_y +gira_y_mais();
                    } else if(pos_y>posicoes[i].y) {
                        pos_y = pos_y +gira_y_menos();
                    } 

                    // Movimenta x até a posição salva
                    if(pos_x<posicoes[i].x){
                        pos_x = pos_x +gira_x_mais();
                    } else if(pos_x>posicoes[i].x) {
                        pos_x = pos_x +gira_x_menos();
                    } 
                    
                    if (pos_y==posicoes[i].y && pos_x==posicoes[i].x) {
                        pipeta = 0;
                        delay(300);
                        pipeta = 1;
                        pipeta_cheia = 0;
                        posicoes[i].vol = posicoes[i].vol--;
                        pipetando_tela(posicoes[i],i+1);
                        wait(2.3);
                        break;
                    }
                    
                    if(emergencia==0){
                        break;
                    }
                }
            }
        }
    }

    PIP = 0;
    for (int i=0;i<9;i++){
        if(posicoes[i].vol>0){
            PIP=1;
        }
    }
    
    if(PIP==0){
        pipetagem_concluida_tela();
        wait(3);
    }

    inicio_tela();
    
}