#ifndef RELOJ_H
#define RELOJ_H

#include <xc.h>

#define _XTAL_FREQ      32000000UL

/* ── Constantes iniciales ─────────────────────────────────────────────────── */
#define HORA_INICIAL    14
#define MINUTO_INICIAL  2
#define SEGUNDO_INICIAL 0

/* ── Botones (activo bajo, pull-ups habilitados) ──────────────────────────── */
#define BTN_UP    (!PORTBbits.RB0)
#define BTN_MODE  (!PORTBbits.RB1)
#define BTN_DOWN  (!PORTBbits.RB2)

/* ── TMR0 precargas (FOSC/4 = 8 MHz, PSC 1:1024 → 7812.5 Hz, 16-bit) ───── */
/* Precarga = 65536 − ticks deseados.  Desborda → PIR0bits.TMR0IF = 1       */
#define TMR0_PRELOAD_2S  (65536u - 15625u)   /* 1024/8MHz*49911 ≈ 2.0 s */
#define TMR0_PRELOAD_6S  (65536u - 46875u)   /* 1024/8MHz*18661 ≈ 6.0 s */

/* ── Tipos ─────────────────────────────────────────────────────────────────── */
typedef enum {
    VISUALIZAR_HORA   = 0,
    CONFIGURAR_ALARMA = 1,
    MAX_MODOS
} modo_t;

typedef enum {
    CONF_HORAS   = 0,
    CONF_MINUTOS = 1,
    ACEPTAR       = 2
} conf_sub_t;

typedef struct {
    unsigned char horas;
    unsigned char minutos;
    unsigned char segundos;
    unsigned char centesimas;
    unsigned int b_24h : 1;
    unsigned int b_alarma : 1;
    unsigned int b_alarma_sonando : 1;
    unsigned int b_cambiando_modo : 1;
    unsigned int b_pulsacion_larga : 1;
    unsigned int b_reiniciar_pantalla : 1;
    unsigned char alarma_horas;
    unsigned char alarma_minutos;
    modo_t        modo;
} reloj_t;

/* ── Variable global (definida en reloj.c) ─────────────────────────────────── */
extern reloj_t reloj;
extern unsigned char  conf_horas_tmp;
extern unsigned char  conf_minutos_tmp;
extern __bit conf_aceptar;
extern conf_sub_t     conf_sub_estado;

/* ── Prototipos ────────────────────────────────────────────────────────────── */
/* Inicializar */
void        iniciar_reloj(void) ;   
void        correr_reloj(void); /* llamar desde main.c cada iteración del loop */

/* TMR0 */
void         tmr0_iniciar_2s(void);
void         tmr0_iniciar_6s(void);
void         tmr0_detener(void);

/* Visualización / modos */
void mostrar_hora_fmt(unsigned char h24, unsigned char m, unsigned char con_seg);
void visualizar_hora(void);
void visualizar_conf_alarma(void);

/* ISR — tick de centésimas + verificación de alarma */
void reloj_tick(void);

#endif /* RELOJ_H */
