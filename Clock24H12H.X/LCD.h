#ifndef LCD_H
#define	LCD_H

/*==============================================================================
 *  LIBRERÍA LCD 16x2 — Interfaz 4 bits
 *  Desarrollo original : Prof. Sergio Salas
 *  Curso: Microcontroladores
 *  Universidad Peruana de Ciencias Aplicadas
 *  Modificaciones      : Kalun Jose Lau Gan, remgo696
 *==============================================================================
 *
 *  CONFIGURACIÓN DE PUERTOS
 *  ────────────────────────
 *  Modifica LCD_CTRL_PORT y LCD_DATA_PORT para elegir en qué puerto van
 *  los pines de control y los de datos respectivamente.
 *
 *  Control: RS(bit0), RW(bit1), E(bit2) en LCD_CTRL_PORT
 *  Datos: D4(bit4), D5(bit5), D6(bit6), D7(bit7) en LCD_DATA_PORT
 *
 *  ⚠ No usar pines RD0, RD1 (18FQ10) o RF0, RF1 (18FQ43) conectados al debugger.
 *
 *============================================================================*/

/* ── Configuración usuario ─────────────────────────────────────────────────── */

#define LCD_CTRL_PORT   C           /* Puerto de control: RS, RW, E           */
#define LCD_DATA_PORT   D           /* Puerto de datos  : D4, D5, D6, D7     */
#define _XTAL_FREQ      32000000UL  /* Frecuencia del oscilador (Hz)          */

/* ── Constantes ────────────────────────────────────────────────────────────── */

#include <stdbool.h>                    /* true / false                        */

/* ── Macros internas (NO modificar) ────────────────────────────────────────── */

/* Doble indirección para token-pasting con argumentos de macro */
#define _LCD_PASTE(a, b)            a##b
#define _LCD_EXPAND(a, b)           _LCD_PASTE(a, b)
#define _LCD_LATBIT2(p, n)          LAT##p##bits.LAT##p##n
#define _LCD_LATBIT(p, n)           _LCD_LATBIT2(p, n)

/* Registros del puerto de control */
#define LCD_LAT_CTRL                _LCD_EXPAND(LAT,   LCD_CTRL_PORT)
#define LCD_TRIS_CTRL               _LCD_EXPAND(TRIS,  LCD_CTRL_PORT)
#define LCD_ANSEL_CTRL              _LCD_EXPAND(ANSEL, LCD_CTRL_PORT)

/* Registros del puerto de datos */
#define LCD_LAT_DATA                _LCD_EXPAND(LAT,   LCD_DATA_PORT)
#define LCD_TRIS_DATA               _LCD_EXPAND(TRIS,  LCD_DATA_PORT)
#define LCD_PORT_DATA               _LCD_EXPAND(PORT,  LCD_DATA_PORT)
#define LCD_ANSEL_DATA              _LCD_EXPAND(ANSEL, LCD_DATA_PORT)

/* Pines de control */
#define RS  _LCD_LATBIT(LCD_CTRL_PORT, 0)
#define RW  _LCD_LATBIT(LCD_CTRL_PORT, 1)
#define E   _LCD_LATBIT(LCD_CTRL_PORT, 2)

/* ── Prototipos ────────────────────────────────────────────────────────────── */

void LCD_INIT(void);
void BORRAR_LCD(void);
void POS_CURSOR(unsigned char fila, unsigned char columna);
void CURSOR_HOME(void);
void CURSOR_ONOFF(bool activar);
void CURSOR_SHIFTLEFT(void);
void CURSOR_SHIFTRIGHT(void);
void BLINK_CURSOR(bool activar);
void DISPLAY_ONOFF(bool activar);
void DISPLAY_SHIFTLEFT(void);
void DISPLAY_SHIFTRIGHT(void);
void ENVIA_CHAR(unsigned char dato);
void ENVIA_NIBBLE(unsigned char dato);
void ENVIA_LCD_CMD(unsigned char dato);
void LEER_LCD(void);
void LCD_CONFIG(void);
void ESCRIBE_MENSAJE(const char *cadena, unsigned char tam);
void ESCRIBE_MENSAJE2(const char *cadena);
void LCD_ESCRIBE_VAR_CHAR(unsigned char numero, unsigned char n_digitos);
void LCD_ESCRIBE_VAR_INT(unsigned int numero, unsigned char n_digitos);
void LCD_ESCRIBE_DECIMAL_VAR_INT(unsigned int numero, unsigned char n_digitos, unsigned char punto);
void LCD_CHAR_GRADO(void);
void LCD_VARCHAR_BITS(unsigned char dato);
void GENERACARACTER(const unsigned char *vector, unsigned char pos);

#endif	/* LCD_LIB_H_ */