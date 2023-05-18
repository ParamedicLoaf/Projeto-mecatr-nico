//**********************************cabeçalho******************************************//

#include "Motor.h"
#include "mbed.h"


//Motor y
DigitalOut clk_y(PB_13);
DigitalOut direcao_y(PB_14);
InterruptIn fdc1_y(PB_15);
InterruptIn fdc2_y(PB_1);

//Motor x 
DigitalOut clk_x(PC_12);
DigitalOut direcao_x(PC_10);
InterruptIn fdc1_x(PB_12);
InterruptIn fdc2_x(PB_11);

//controlador de velocidade
int tempo_y = 700;
int tempo_x = 500;


//acionamentos do motor y___________________________________________________________________
int gira_y_mais(){

    if (fdc1_y == 0){ //só roda se o fim de curso não estiver acionado
        
        direcao_y = 1;
        clk_y =1;
        wait_us(tempo_y);
        clk_y = 0;
        wait_us(tempo_y);
        
        return 1;
    
    } else {
        return 0;
    }
    
}

int gira_y_menos(){

    if (fdc2_y == 0){ //só roda se o fim de curso não estiver acionado

        direcao_y = 0;
        clk_y =1;
        wait_us(tempo_y);
        clk_y = 0;
        wait_us(tempo_y);
        
        return -1;

    } else {
        return 0;
    }

}

void stop_y(){
    clk_y = 0;
}



//acionamentos do motor x_______________________________________________________________________

int gira_x_mais(){

    if (fdc1_x == 0){ //só roda se o fim de curso não estiver acionado
        
        direcao_x = 0;
        clk_x =1;
        wait_us(tempo_x);
        clk_x = 0;
        wait_us(tempo_x);
        
        return 1;
    
    } else {
        return 0;
    }
    
}

int gira_x_menos(){

    if (fdc2_x == 0){ //só roda se o fim de curso não estiver acionado

        direcao_x = 1;
        clk_x =1;
        wait_us(tempo_x);
        clk_x = 0;
        wait_us(tempo_x);
        
        return -1;

    } else {
        return 0;
    }

}

void stop_x(){
    clk_x = 0;
}