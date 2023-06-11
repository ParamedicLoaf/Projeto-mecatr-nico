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
DigitalIn confirma(PC_6);
DigitalIn baixo(PC_4);
DigitalIn cima(PB_13);
DigitalIn voltar(PB_7);
AnalogIn EixoYJoyStick(PC_2);
AnalogIn EixoXJoyStick(PC_3);
InterruptIn emergencia(PC_5);

// pipeta
DigitalOut pipeta(PD_2);

//Motor Y
InterruptIn fdc2_y_(PB_1);

//Motor X
InterruptIn fdc2_x_(PB_11);

//Motor Z
InterruptIn fdc2_z_(PB_15);

// variaveis auxiliares
bool REF = 0;
bool flag_emergencia = 1;
bool PIP = 0;
bool pipeta_cheia = 0;
int cursor=1;
int cursor_pos=0;
int volume;

//variáveis de posição e joystick
int joy_y;
int pos_y;
int joy_x;
int pos_x;
int pos_z;

// variaveis de movimentação
bool Y_MAIS=0;
bool Y_MENOS=0;
bool X_MAIS=0;
bool X_MENOS=0;
bool Z_MAIS=0;
bool Z_MENOS=0;

// Temporizadores
Timer display;
Timer debounce;
Ticker vel_x;
Ticker vel_y;
Ticker vel_z;
float tempo_x= 1200;
float tempo_y= 1400;
float tempo_z= 2000;

//Estrutura de dados para captura de posição e volume
struct pos {
  int y;
  int x;
  int z;
  int vol;
};

struct pega {
  int y;
  int x;
  int z;
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
pos posicoes_backup[9] = {posicao_1,posicao_2,posicao_3,posicao_4,posicao_5,posicao_6,posicao_7,posicao_8,posicao_9};



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

void pipetando_tela(pos posicao, int i){
    tft.setCursor(10, 0); // Orientação X,Y
    tft.printf("Posicao %d",i);
    tft.setCursor(10, 30);
    tft.fillRect(50,30,250,30,BLACK);
    tft.printf("Faltam %d ml",posicao.vol);
}

//****************************************************************************//

// PROGRAMA PRINCIPAL ____________________________________________________________

void setup(void){  //roda apenas uma vez

    //configs da tela
    tft.reset();
    tft.begin();
    tft.setRotation(Orientation);
    tft.fillScreen(BLACK);  // Fundo do Display
    tft.setTextColor(CYAN);
    tft.setTextSize(3);
    delay(1000);

    pipeta = 1; //abre o relay

    //inicia o debouncing
    debounce.start();
    display.start();

    //interrupções
    emergencia.fall(&desastre);

    //associa as movimentações com as velocidades
    vel_x.attach_us(mov_x,tempo_x);
    vel_y.attach_us(mov_y,tempo_y);
    vel_z.attach_us(mov_z,tempo_z);

    
    inicio_tela();
    
}

void loop(){

    // normaliza a flag emergencia
    flag_emergencia = 1;

    // Para os motores
    X_MENOS=0;
    X_MAIS=0;
    Y_MENOS=0;
    Y_MAIS=0;
    Z_MENOS=0;
    Z_MAIS=0;

    // Lê o joystick 
    joy_y = EixoYJoyStick.read() * 1000;
    joy_x = EixoXJoyStick.read() * 1000;
    joy_y = 1000-joy_y;
    joy_x = 1000-joy_x;

    //controle de cursores
    if (joy_y<400 || baixo){
        cursor++;
        if (cursor>3){
            cursor = 1;
        }
        inicio_tela();

        delay(300);

    } else if (joy_y>600 || cima){
        cursor--;
        if(cursor<1){
            cursor=3;
        }

        inicio_tela();
        delay(300);

    }

    if (confirma){ // função selecionada

        switch(cursor){
//______________REFERENCIAMENTO_____________________________________________________________________
            case 1:
                referenciamento_tela();
                wait(0.5);
                while(1){
                    if(confirma){ //confirmando referencia
                        estado_ref();
                        delay(300);
                        inicio_tela();
                        break;
                    }
                    if(voltar){
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

void desastre(){ //quando o emergência é acionado
        //para os motores
        X_MENOS=0;
        X_MAIS=0;
        Y_MENOS=0;
        Y_MAIS=0;
        Z_MENOS=0;
        Z_MAIS=0;

        REF = 0; //perde a referencia
        flag_emergencia = 0; //aciona a flag emergencia
        emergencia_tela(); 
        while (emergencia == 0){} //enquanto etiver apertado nada acontecee

        inicio_tela();

}

void estado_ref(){ //para referenciar
    if (debounce.read_ms() >30 && REF==0 && emergencia==1){ //debouncing
        referencia(); //função de referencia

        if (fdc2_y_==1 && fdc2_x_==1 && fdc2_z_==1){REF = 1;} //se a função for concluida, REF =1
        
        // zera as posições
        pos_y = 0;
        pos_x = 0;
        pos_z = 0;
        
        inicio_tela();
    }

    debounce.reset();
}

void referencia(){

    referenciando_tela();

    //referenciamento de Z
    while(fdc2_z_==0 && flag_emergencia==1){ //roda até bater no fim de curso 2
        
        Z_MENOS=1;

        if(emergencia==0){
            break;
        }

    }
    Z_MENOS = 0;

    //referenciamento de X e Y
    while((fdc2_y_==0||fdc2_x_==0) && flag_emergencia==1){ //roda até bater no fim de curso 2
        
        Y_MENOS=1;
        X_MENOS=1;

        if(emergencia==0){
            break;
        }

    }
    Y_MENOS = 0;
    X_MENOS = 0;
    
}

void seleciona_posicao(){
    lista_pos_tela();
    delay(300);

    while(flag_emergencia){
        
        //Menu de posições
        if(voltar){
            inicio_tela();
            for (int i=0;i<9;i++){
                posicoes_backup[i] = posicoes[i];
            }
            delay(300);
            break;
        }

        joy_y = EixoYJoyStick.read() * 1000;
        joy_y = 1000-joy_y;

        if (joy_y<400  || baixo){
            cursor_pos++;
            if (cursor_pos>9){
                cursor_pos = 0;
            }
            lista_pos_tela();

            delay(300);

        } else if (joy_y>600  || cima){
            cursor_pos--;
            if(cursor_pos<0){
                cursor_pos=9;
            }

            lista_pos_tela();
            delay(300);

        }

        if (confirma){ //movimentando pos específica
            delay(300);
            //JOG
            while (REF == 1){

                if(voltar){
                    X_MENOS=0;
                    X_MAIS=0;
                    Y_MENOS=0;
                    Y_MAIS=0;
                    Z_MENOS=0;
                    Z_MAIS=0;
                    lista_pos_tela();
                    delay(300);
                    break;
                }

                joy_y = EixoYJoyStick.read() * 1000;
                joy_x = EixoXJoyStick.read() * 1000;

                joy_y = 1000-joy_y;
                joy_x = 1000-joy_x;

                // gira o motor Y de acordo com a leitura do Joystick
                if (joy_y>600){
                    Y_MAIS=0;
                    Y_MENOS=1;
                    //pos_y = pos_y + gira_y_menos();
                } else if (joy_y<400){
                    Y_MAIS=1;
                    Y_MENOS=0;
                    //pos_y = pos_y + gira_y_mais();
                } else {
                    Y_MAIS=0;
                    Y_MENOS=0;
                    //stop_y();
                }


                if((joy_y<600 && joy_y>400)&&(joy_x<600 && joy_x>400)){ //se estiver parado
                    // a cada 2 segundos, printa a posição no display:
                    if (display.read_ms()>2000){ 
                        print_posicao();
                        display.reset();
                    }
                }

                // gira o motor X de acordo com a leitura do Joystick
                if (joy_x>600){
                    X_MAIS=1;
                    X_MENOS=0;
        
                } else if (joy_x<400){
                    X_MAIS=0;
                    X_MENOS=1;
                    
                } else {
                    X_MAIS=0;
                    X_MENOS=0;
                    
                }

                // gira o motor Z de acordo com a leitura dos botoes
                if (baixo){
                    Z_MAIS=1;
                    Z_MENOS=0;
                    
                } else if (cima){
                    Z_MAIS=0;
                    Z_MENOS=1;
                    
                } else {
                    Z_MAIS=0;
                    Z_MENOS=0;
                    
                }

                if(confirma){ //confirmar posições
                    X_MENOS=0;
                    X_MAIS=0;
                    Y_MENOS=0;
                    Y_MAIS=0;
                    Z_MENOS=0;
                    Z_MAIS=0;

                    delay(300);
                    switch(cursor_pos){
                        case 0: //caso seja posição pega
                            pos_pega.y = pos_y;
                            pos_pega.x = pos_x;
                            pos_pega.z = pos_z;
                            break;
                        default: //caso seja uma posição normal
                            posicoes[cursor_pos-1].y=pos_y;
                            posicoes[cursor_pos-1].x=pos_x;
                            posicoes[cursor_pos-1].z=pos_z;

                            pega_volume_tela();
                            while(flag_emergencia){ //pega o volume

                                joy_y = EixoYJoyStick.read() * 1000;
                                joy_y = 1000-joy_y;
                                
                                if (joy_y>600  || cima){
            
                                    volume++;
                                    pega_volume_tela();
                                    delay(300);

                                } else if (joy_y<400  || baixo){

                                    volume--;
                                    if(volume<0){
                                        volume=0;
                                    }
                                    pega_volume_tela();
                                    delay(300);

                                }

                                if (confirma){ 
                                    delay(300);
                                    posicoes[cursor_pos-1].vol=volume;
                                    if(pos_pega.z>0){PIP = 1;}
                                    break;
                                }
                                if(voltar){
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

    for (int i=0;i<9;i++){ //percorre todas as posições

        limpa_tela();

        pipetando_tela(posicoes[i],i+1);

        while(posicoes[i].vol>0 && flag_emergencia){ 

            //se a pipeta estiver vazia, vai até posição pega
            if (pipeta_cheia==0 && flag_emergencia){ 
                
                while(REF && flag_emergencia){
                    // Movimenta z até a posição mais alta
                    if(pos_z<10){
                        Z_MAIS=1;
                        Z_MENOS=0; 
                    } else if(pos_z>10) {
                        Z_MAIS=0;
                        Z_MENOS=1;
                    } else {
                        Z_MAIS=0;
                        Z_MENOS=0;
                    }

                    if(pos_z==10 || emergencia==0 || flag_emergencia==0){
                        Z_MAIS=0;
                        Z_MENOS=0;
                        break;
                    }
                }

                while(REF && flag_emergencia){

                    // Movimenta y até a posição de pega
                    if(pos_y<pos_pega.y){
                        Y_MAIS=1;
                        Y_MENOS=0;
                        
                    } else if(pos_y>pos_pega.y) {
                        Y_MAIS=0;
                        Y_MENOS=1;
                        
                    } else {
                        Y_MAIS=0;
                        Y_MENOS=0;
                    }

                    // Movimenta x até a posição de pega
                    if(pos_x<pos_pega.x){
                        X_MAIS=1;
                        X_MENOS=0;
                        
                    } else if(pos_x>pos_pega.x) {
                        X_MAIS=0;
                        X_MENOS=1;
                        
                    } else {
                        X_MAIS=0;
                        X_MENOS=0;
                    }

                    // se X e Y estiverem na posição certa, movimentar Z
                    if(pos_x==pos_pega.x && pos_y==pos_pega.y){
                        while(REF && flag_emergencia){
                            // Movimenta z até a posição de pega
                            if(pos_z<pos_pega.z){
                                Z_MAIS=1;
                                Z_MENOS=0;
                                
                            } else if(pos_z>pos_pega.z) {
                                Z_MAIS=0;
                                Z_MENOS=1;
                                
                            } else {
                                Z_MAIS=0;
                                Z_MENOS=0;
                            }

                            if(pos_z==pos_pega.z || emergencia==0 || flag_emergencia==0){
                                Z_MAIS=0;
                                Z_MENOS=0;
                                break;
                            }
                        }

                        if (flag_emergencia){

                            //aciona a pipeta e pega o líquido
                            pipeta = 0;
                            delay(100);
                            pipeta = 1;
                            pipeta_cheia = 1;
                            wait(1.6);
                            break;
                        }
                    }

                    if(emergencia==0 || flag_emergencia==0){
                        break;
                    }

                }
            } else {

                // caso a pipeta esteja cheia, movimenta até a posição atual

                while(REF && flag_emergencia){
                    // Movimenta z até a posição mais alta
                    if(pos_z<10){
                        Z_MAIS=1;
                        Z_MENOS=0; 
                    } else if(pos_z>10) {
                        Z_MAIS=0;
                        Z_MENOS=1;
                    } else {
                        Z_MAIS=0;
                        Z_MENOS=0;
                    }

                    if(pos_z==10 || emergencia==0 || flag_emergencia==0){
                        Z_MAIS=0;
                        Z_MENOS=0;
                        break;
                    }
                }

                while(REF && flag_emergencia){

                    // Movimenta y até a posição salva
                    if(pos_y<posicoes[i].y && flag_emergencia){
                        Y_MAIS=1;
                        Y_MENOS=0;
                        
                    } else if(pos_y>posicoes[i].y && flag_emergencia) {
                        Y_MAIS=0;
                        Y_MENOS=1;
                        
                    } else {
                        Y_MAIS=0;
                        Y_MENOS=0;
                    }

                    // Movimenta x até a posição salva
                    if(pos_x<posicoes[i].x){
                        X_MAIS=1;
                        X_MENOS=0;
                        //pos_x = pos_x +gira_x_mais();
                    } else if(pos_x>posicoes[i].x) {
                        X_MAIS=0;
                        X_MENOS=1;
                        //pos_x = pos_x +gira_x_menos();
                    } else {
                        X_MAIS=0;
                        X_MENOS=0;
                    }

                    //Se X e Y estiverem na posição certa, movimenta Z
                    if (pos_y==posicoes[i].y && pos_x==posicoes[i].x) {
                        while(REF && flag_emergencia){
                            // Movimenta z até a posição salva
                            if(pos_z<posicoes[i].z){
                                Z_MAIS=1;
                                Z_MENOS=0; 
                            } else if(pos_z>posicoes[i].z){
                                Z_MAIS=0;
                                Z_MENOS=1;
                            } else {
                                Z_MAIS=0;
                                Z_MENOS=0;
                            }

                            if(pos_z==posicoes[i].z || emergencia==0 || flag_emergencia==0){
                                Z_MAIS=0;
                                Z_MENOS=0;
                                break;
                            }
                        }

                        if (flag_emergencia){

                            //aciona a pipeta e solta o líquido
                            pipeta = 0;
                            delay(100);
                            pipeta = 1;
                            pipeta_cheia = 0;
                            posicoes[i].vol = posicoes[i].vol--;
                            pipetando_tela(posicoes[i],i+1);
                            wait(2.3);
                            break;
                        }
                    }
                    
                    if(emergencia==0 || flag_emergencia==0){
                        break;
                    }
                }
            }
        }
    }

    PIP = 0;
    for (int i=0;i<9;i++){ //checa se falta algum volume
        if(posicoes[i].vol>0){
            PIP=1;
        }
    }
    
    if(PIP==0){ // se não sobrar nenhum volume, mostrar tela concluida
        pipetagem_concluida_tela();
        for (int i=0;i<9;i++){
            posicoes[i] = posicoes_backup[i];
        }
        for (int i=0;i<9;i++){
            if(posicoes[i].vol>0){
                PIP=1;
            }
        }
        wait(3);
    }

    inicio_tela();
    
}

//*****************************************************************************************//
void mov_y(){ //movimentação de Y
    if (Y_MENOS && flag_emergencia){
        pos_y = pos_y + gira_y_menos();
    } else if (Y_MAIS && flag_emergencia){
        pos_y = pos_y + gira_y_mais();
    } else {
        stop_y();
    }
}

void mov_x(){ //movimentação de X
    if (X_MENOS && flag_emergencia){
        pos_x = pos_x + gira_x_menos();
    } else if (X_MAIS && flag_emergencia){
        pos_x = pos_x + gira_x_mais();
    } else {
        stop_x();
    }
}

void mov_z(){ //movimentação de Z
    if (Z_MENOS && flag_emergencia){
        pos_z = pos_z + gira_z_menos();
    } else if (Z_MAIS && flag_emergencia && pos_z*5/200<=101){
        pos_z = pos_z + gira_z_mais();
    } else {
        stop_z();
    }
}

// TELAS DISPLAY__________________________________________________________________________________

void limpa_tela(){
    tft.fillScreen(BLACK);
}

void pipetagem_concluida_tela(){ //tela de conclusão
    tft.fillScreen(BLACK);
    tft.setCursor(50, 200); // Orientação X,Y
    tft.setTextSize(2);
    tft.printf("PIPETAGEM CONCLUIDA");
    tft.setTextSize(3);
}

void print_posicao(){ //indica posição atual durante jog

    tft.fillScreen(BLACK);
    tft.setCursor(10, 10); // Orientação X,Y
    tft.printf("x = %d mm",pos_x*3/200);
    tft.setCursor(10, 40); 
    tft.printf("y = %d mm",pos_y*3/200);
    tft.setCursor(10, 70);
    tft.printf("z = %d mm",pos_z*5/200);
    tft.setCursor(10, 110);

}

void pega_volume_tela(){ //tela que pede o volume a ser pipetado

    tft.fillScreen(BLACK);
    tft.setCursor(10, 10); // Orientação X,Y
    tft.printf("x = %d mm",pos_x*3/200);
    tft.setCursor(10, 40); // Orientação X,Y
    tft.printf("y = %d mm",pos_y*3/200);
    tft.setCursor(10, 70);
    tft.printf("z = %d mm",pos_z*3/200);
    tft.setCursor(10, 110);
    tft.printf("%d ml",volume);
    tft.setCursor(10, 160);
    tft.setTextSize(2);
    tft.print("Selecione o volume");
    tft.setTextSize(3);
}

void inicio_tela(){ // menu de início, com 3 opções

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

void referenciamento_tela(){ //tela antes do referenciamento

    tft.fillScreen(BLACK);
    tft.setTextColor(CYAN);
    tft.setCursor(10, 85); // Orientação X,Y
    tft.printf("\rPor favor\naperte");
    tft.setTextColor(GREEN);
    tft.printf(" confirma \n");
    tft.setTextColor(CYAN);
    tft.printf("para referenciar");
}

void referenciando_tela(){ //tela durante o referenciamento
    tft.fillScreen(BLACK);
    tft.setCursor(50, 200); // Orientação X,Y
    tft.setTextSize(2);
    tft.printf("REFERENCIANDO...");
    tft.setTextSize(3);
}

void emergencia_tela(){ //tela durante o emergência

    tft.fillScreen(BLACK);
    tft.setCursor(80, 10); // Orientação X,Y
    tft.setTextColor(RED);
    tft.print("\rEMERGENCIA\n");
    tft.setTextColor(CYAN);
    tft.println("\n\nDesative o botao\nquando for seguro");
}

void lista_pos_tela(){ //menu com a posição pega + 9 posições

    tft.fillScreen(BLACK);
    tft.setTextColor(RED);
    tft.setCursor(10, 10);
    tft.setTextSize(2);
    if(pos_pega.z>0){tft.setTextColor(CYAN);}
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
    
    tft.setCursor(220, 90);
    tft.setTextSize(1);
    tft.setTextColor(CYAN);
    if (PIP == 0){
        tft.print("Preencha a");
        tft.setCursor(220, 110);
        tft.print("posicao pega");
        tft.setCursor(220, 130);
        tft.print("e 1 volume");
    } else {
        tft.print("Pressione");
        tft.setCursor(220, 110);
        tft.print("voltar");
        tft.setCursor(220, 130);
        tft.print("para pipetar");
    }
    tft.setTextSize(3);
    tft.drawRoundRect(5,5+20*cursor_pos,210,24,1,WHITE);
}