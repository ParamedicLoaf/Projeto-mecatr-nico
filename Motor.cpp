//**********************************cabeçalho******************************************//

#include "Motor.h"
#include "mbed.h"


//Motor y
DigitalOut clk_y(PB_13);
DigitalOut direcao_y(PB_14);
InterruptIn fdc1_y(PB_15);
InterruptIn fdc2_y(PB_1);

//controlador de velocidade y
int tempo_y = 500;

//acionamentos do motor y
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




