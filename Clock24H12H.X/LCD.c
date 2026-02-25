#include <xc.h>
#include "LCD.h"
#include <string.h>

void POS_CURSOR(unsigned char fila,unsigned char columna)
{
	if(fila == 1)
	{
		ENVIA_LCD_CMD(0x80+columna);
	}
	else if(fila == 2)
	{
		ENVIA_LCD_CMD(0xC0+columna);
	}
	else if(fila == 3)
	{
		ENVIA_LCD_CMD(0x94+columna);
	}
	else if(fila == 4)
	{
		ENVIA_LCD_CMD(0xD4+columna);
	}
}

void BLINK_CURSOR(bool activar)
{
	if(activar) ENVIA_LCD_CMD(0x0F);
	else        ENVIA_LCD_CMD(0x0E);
}

void DISPLAY_ONOFF(bool activar)
{
	if(activar) ENVIA_LCD_CMD(0x0F);
	else        ENVIA_LCD_CMD(0x08);
}

void CURSOR_HOME(void)
{
	ENVIA_LCD_CMD(0x02);
}

void CURSOR_SHIFTLEFT(void)
{
	ENVIA_LCD_CMD(0x10);
}

void CURSOR_SHIFTRIGHT(void)
{
	ENVIA_LCD_CMD(0x14);
}

void DISPLAY_SHIFTLEFT(void)
{
	ENVIA_LCD_CMD(0x18);
}

void DISPLAY_SHIFTRIGHT(void)
{
	ENVIA_LCD_CMD(0x1C);
}

void CURSOR_ONOFF(bool activar)
{
	if(activar) ENVIA_LCD_CMD(0x0E);
	else        ENVIA_LCD_CMD(0x0C);
}

void ESCRIBE_MENSAJE(const char *cadena,unsigned char tam)
{
	unsigned char i = 0;
	for(i = 0; i<tam; i++)
	{
		ENVIA_CHAR(cadena[i]);
	}
}

void ESCRIBE_MENSAJE2(const char *cadena)
{
    unsigned char tam;
    tam = (unsigned char)strlen(cadena);
	unsigned char i = 0;
	for(i = 0; i<tam; i++)
	{
		ENVIA_CHAR(cadena[i]);
	}
}

void ENVIA_CHAR(unsigned char dato)
{
	unsigned char aux;
	RS = 1;
	LEER_LCD();
	LCD_TRIS_CTRL &= 0xF8;
	LCD_TRIS_DATA &= 0x0F;
    __delay_us(100);
	RW = 0;
	E = 0;
	RS = 1;
	aux = dato & 0xF0;
	ENVIA_NIBBLE(aux);
	aux = (unsigned char)(dato << 4);
	ENVIA_NIBBLE(aux);	
}

void BORRAR_LCD(void)
{
	ENVIA_LCD_CMD(0x01);
}

void LCD_CONFIG(void)
{
	RS = 0;
	RW = 0;
	ENVIA_NIBBLE(0x30);
    __delay_ms(2);
	ENVIA_NIBBLE(0x30);
    __delay_us(100);
    ENVIA_NIBBLE(0x30);
	ENVIA_NIBBLE(0x20);
	ENVIA_LCD_CMD(0x01);
	ENVIA_LCD_CMD(0x28);
	ENVIA_LCD_CMD(0x0F);
	ENVIA_LCD_CMD(0x06);
	ENVIA_LCD_CMD(0x01);
}

void ENVIA_NIBBLE(unsigned char dato)
{
	LCD_LAT_DATA &= 0x0F;
	dato &= 0xF0;
	LCD_LAT_DATA |= dato;
	E = 1;
    __delay_us(100);
	E = 0;
}

void ENVIA_LCD_CMD(unsigned char dato)
{
	unsigned char aux;
	RS = 0;
	LEER_LCD();
	LCD_TRIS_CTRL &= 0xF8;
	LCD_TRIS_DATA &= 0x0F;
    __delay_us(100);
    RW = 0;
	E = 0;
	RS = 0;
	aux = dato & 0xF0;
	ENVIA_NIBBLE(aux);
	aux = (unsigned char)(dato << 4);
	ENVIA_NIBBLE(aux);
}

void LEER_LCD(void)
{
	unsigned char aux;
	LCD_TRIS_DATA |= 0xF0;
	LCD_TRIS_CTRL &= 0xF8;
	RS = 0;
	RW = 1;
	E = 1;
    __delay_us(100);
	aux = LCD_PORT_DATA;
	E = 0;
    __delay_us(100);
	E = 1;
    __delay_us(100);
	E = 0;
	aux = aux & 0x80;
	while(aux == 0x80)
        {
            E = 1;
            __delay_us(100);
            aux = LCD_PORT_DATA;
            E = 0;
            __delay_us(100);
            E = 1;
            __delay_us(100);
            E = 0;
            aux = aux & 0x80;
	}
}

void GENERACARACTER(const unsigned char *vector,unsigned char pos)
{
	unsigned char i;
	ENVIA_LCD_CMD(0x40+8*pos);//Dirección de la CGRAM +
	for(i=0;i<8;i++)			 //offset de posición de caracter	
	{
		ENVIA_CHAR(vector[i]);
	}
	ENVIA_LCD_CMD(0x80);	//Dirección de la DDRAM
}

void LCD_INIT(void){
    LCD_TRIS_CTRL &= 0xF8;
    LCD_TRIS_DATA &= 0x0F;
    LCD_ANSEL_CTRL &= 0xF8;
    LCD_ANSEL_DATA &= 0x0F;
    __delay_ms(15);
    LCD_CONFIG();
    __delay_ms(15);
    BORRAR_LCD();
    CURSOR_HOME();
    CURSOR_ONOFF(false);
}

void LCD_ESCRIBE_VAR_CHAR(unsigned char numero, unsigned char n_digitos){
    unsigned char i;
    unsigned int  divisor = 1;
    for(i = 1; i < n_digitos; i++) divisor *= 10;
    for(i = 0; i < n_digitos; i++){
        ENVIA_CHAR((unsigned char)((numero / divisor) % 10) + 0x30);
        divisor /= 10;
    }
}

void LCD_ESCRIBE_VAR_INT(unsigned int numero, unsigned char n_digitos){
    unsigned char i;
    unsigned int  divisor = 1;
    for(i = 1; i < n_digitos; i++) divisor *= 10;
    for(i = 0; i < n_digitos; i++){
        ENVIA_CHAR((unsigned char)((numero / divisor) % 10) + 0x30);
        divisor /= 10;
    }
}

void LCD_ESCRIBE_DECIMAL_VAR_INT(unsigned int numero, unsigned char n_digitos, unsigned char punto){
	unsigned char i;
	unsigned int  divisor = 1;
    for(i = 1; i < n_digitos; i++) divisor *= 10;
    for(i = 0; i < n_digitos; i++){
        ENVIA_CHAR((unsigned char)((numero / divisor) % 10) + 0x30);
        divisor /= 10;
		if(i == punto) ENVIA_CHAR('.');
    }
}

void LCD_CHAR_GRADO(void){
    ENVIA_CHAR(0xDF);
}

void LCD_VARCHAR_BITS(unsigned char dato){
    unsigned char x_var;
    for(x_var=0;x_var<8;x_var++){
        if (((dato >> (7 - x_var)) & 0x01) == 1){
            ENVIA_CHAR('1');
        }
        else{
            ENVIA_CHAR('0');
        }
    }
}