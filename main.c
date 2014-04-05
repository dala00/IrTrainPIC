#include <xc.h>

#pragma config FOSC = INTRCIO, WDTE = OFF, PWRTE = ON, MCLRE = OFF

#define IR GPIO3
#define LED GPIO2
#define MOTOR1 GPIO4
#define MOTOR2 GPIO5
#define TIMEOUT 27500
#define _XTAL_FREQ 4000000 // delay用に必要(クロック32MHzを指定)


#define __delay(x) _delay((unsigned long)((x)))
#define __delay_us(x) _delay((unsigned long)((x)*(_XTAL_FREQ/4000000UL)))
#define __delay_ms(x) _delay((unsigned long)((x)*(_XTAL_FREQ/4000UL)))

char lastInput = 1;
char isSony = 0;
char data = 0;
unsigned long time = 0;
unsigned char inputCount = 0;
unsigned char dataCount = 0;

void setData(int no, char row) {
    //sonyは7bitなのでそれ以外は無視
    if (no >= 7) return;
    data |= row << no;
}

void motorGo() {
    __delay_ms(1);
    LED = 1;
    MOTOR1 = 1;
    MOTOR2 = 0;
}

void motorBack() {
    __delay_ms(1);
    LED = 1;
    MOTOR1 = 0;
    MOTOR2 = 1;
}

void motorStop() {
    __delay_ms(1);
    LED = 0;
    MOTOR1 = 1;
    MOTOR2 = 1;
}

main()	{
    GPIO = 0;
    CMCON = 0x07;		// コンパレータ未使用
    TRISIO = 0b00001000;		// GP4:in
    //プリスケーラ8=8μS…カウント75で600μ(1T)…だけどなか64くらいっぽい…
    OPTION_REG = 0b10000010;
    TMR0 = 0;
    ANSEL = 0b00110000;

    char val;

    while(1) {
        val = IR;
        while (val == lastInput) {
            if (TMR0 > 200) {
                time += TMR0;
                TMR0 = 0;
            }
            if (time + TMR0 > TIMEOUT) {
                if (inputCount > 2 && isSony) {
                    switch (data) {
                    case 1:
                        motorGo();
                        break;
                    case 4:
                        motorStop();
                        break;
                    case 7:
                        motorBack();
                        break;
                    }
                }
                isSony = 0;
                inputCount = 0;
                dataCount = 0;
                data = 0;
            }
            val = IR;
        }

        time += TMR0;
        inputCount++;
        if (inputCount == 2) {
            if (192 < time && time < 320) {
                isSony = 1;
            }
        } else if (inputCount > 2) {
            if (isSony && !(inputCount % 2)) {
                if (time < 100) {
                    setData(dataCount, 0);
                } else {
                    setData(dataCount, 1);
                }
                dataCount++;
            }
        }
        lastInput = val;
        time = 0;
        TMR0 = 0;
    }
}
