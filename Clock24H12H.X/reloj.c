#include <xc.h>
#include "reloj.h"
#include "LCD.h"
#define _XTAL_FREQ 32000000UL

/* ── Variables globales ────────────────────────────────────────────────────── */
reloj_t reloj = {
    HORA_INICIAL, MINUTO_INICIAL, SEGUNDO_INICIAL, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    VISUALIZAR_HORA
};

unsigned char  conf_horas_tmp;
unsigned char  conf_minutos_tmp;
conf_sub_t     conf_sub_estado;
__bit conf_aceptar;

/* ══════════════════════════════════════════════════════════════════════════════
 *  TMR0 helpers — usa desbordamiento (TMR0IF) con precarga
 * ══════════════════════════════════════════════════════════════════════════════ */
static void tmr0_cargar(unsigned int precarga) {
    T0CON0bits.T0EN = 0;
    PIR0bits.TMR0IF = 0;
    TMR0L = (unsigned char)(precarga & 0xFF);
    TMR0H = (unsigned char)(precarga >> 8);
    T0CON0bits.T0EN = 1;
}

void tmr0_iniciar_2s(void) {
    tmr0_cargar(TMR0_PRELOAD_2S);
}

void tmr0_iniciar_6s(void) {
    tmr0_cargar(TMR0_PRELOAD_6S);
}

void tmr0_detener(void) {
    T0CON0bits.T0EN = 0;
    PIR0bits.TMR0IF = 0;
}


/* ══════════════════════════════════════════════════════════════════════════════
 *  Mostrar hora en formato 12/24 h (posición actual del cursor)
 *  Escribe: HH:MM[:SS:CC] AM/PM/HS
 * ══════════════════════════════════════════════════════════════════════════════ */
void mostrar_hora_fmt(unsigned char h24, unsigned char m, unsigned char con_seg) {
    unsigned char hd = h24;
    unsigned char pm = 0;

    if (!reloj.b_24h) {             /* formato 12 h */
        if      (hd == 0)  { hd = 12; pm = 0; }
        else if (hd < 12)  {          pm = 0; }
        else if (hd == 12) {          pm = 1; }
        else               { hd -= 12; pm = 1; }
    }

    ENVIA_CHAR((hd / 10) + '0');
    ENVIA_CHAR((hd % 10) + '0');
    ENVIA_CHAR(':');
    ENVIA_CHAR((m / 10) + '0');
    ENVIA_CHAR((m % 10) + '0');

    if (con_seg) {
        ENVIA_CHAR(':');
        ENVIA_CHAR((reloj.segundos / 10) + '0');
        ENVIA_CHAR((reloj.segundos % 10) + '0');
        ENVIA_CHAR(':');
        ENVIA_CHAR((reloj.centesimas / 10) + '0');
        ENVIA_CHAR((reloj.centesimas % 10) + '0');
    }

    ENVIA_CHAR(' ');
    if (reloj.b_24h) {
        ESCRIBE_MENSAJE("HS", 2);
    } else {
        ESCRIBE_MENSAJE(pm ? "PM" : "AM", 2);
    }
}


static void iniciar_visualizacion(void) {
    BORRAR_LCD();
    if (reloj.modo == CONFIGURAR_ALARMA) {
        conf_minutos_tmp = reloj.minutos + 1;
        conf_horas_tmp   = reloj.horas;
        if (conf_minutos_tmp >= 60) {
            conf_minutos_tmp = 0;
            conf_horas_tmp   = (conf_horas_tmp + 1) % 24;
        }
        conf_sub_estado = CONF_HORAS;
        /* Línea 1 */
        POS_CURSOR(1, 0);
        ESCRIBE_MENSAJE("Conf. alarma:", 13);
        tmr0_iniciar_6s();          /* timeout por inactividad */
    } else {
        /* Línea 1: "Hora actual:"  con 'A' en col 15 si alarma activa */
        POS_CURSOR(1, 0);
        ESCRIBE_MENSAJE("Hora actual:", 12);
        if (reloj.b_alarma) {
            POS_CURSOR(1, 15);
            ENVIA_CHAR('A');
        }
    }
}

/* ══════════════════════════════════════════════════════════════════════════════
 *  Modo VISUALIZAR_HORA
 * ══════════════════════════════════════════════════════════════════════════════ */
void visualizar_hora(void) {

    /* Línea 2: HH:MM:SS:CC AM/PM/HS */
    POS_CURSOR(2, 0);
    mostrar_hora_fmt(reloj.horas, reloj.minutos, 1);

    /* Esperamos a que se suelte el botón MODE para ejecutar la acción */
    if (reloj.b_cambiando_modo && !BTN_MODE) {
        reloj.b_cambiando_modo = 0;
        tmr0_detener();

        if (reloj.b_alarma_sonando) {
            /* Si la alarma está sonando, cualquier botón la detiene */
            reloj.b_alarma_sonando = 0;
            reloj.b_pulsacion_larga = 0;
        } else if (reloj.b_pulsacion_larga) {
            /* Pulsación larga (≥ 2 s) → cambio de modo */
            reloj.b_pulsacion_larga = 0;
            reloj.modo = CONFIGURAR_ALARMA;
            iniciar_visualizacion();
        } else {
            /* Pulsación corta → cambio 24 h / 12 h */
            reloj.b_24h = !reloj.b_24h;
        }
    }
}

/* ══════════════════════════════════════════════════════════════════════════════
 *  Modo CONFIGURAR_ALARMA
 * ══════════════════════════════════════════════════════════════════════════════ */
void visualizar_conf_alarma(void) {

    /* Línea 2: HH:MM AM/PM/HS */
    POS_CURSOR(2, 0);
    mostrar_hora_fmt(conf_horas_tmp, conf_minutos_tmp, 0);

    if (conf_sub_estado == ACEPTAR) {
        POS_CURSOR(2, 9);
        ESCRIBE_MENSAJE(conf_aceptar ? " Acepto" : "Rechazo", 7);
    }

    if (reloj.b_cambiando_modo && !BTN_MODE) {
        reloj.b_cambiando_modo = 0;
        tmr0_detener();

        if (reloj.b_pulsacion_larga) {
            /* Pulsación larga → volver a VISUALIZAR_HORA */
            reloj.b_pulsacion_larga = 0;
            reloj.modo = VISUALIZAR_HORA;
            iniciar_visualizacion();
        } else {
            /* Pulsación corta → "aceptar" sub-estado actual */
            if (conf_sub_estado == CONF_HORAS) {
                conf_sub_estado = CONF_MINUTOS;
                tmr0_iniciar_6s();
            } else if (conf_sub_estado == CONF_MINUTOS) {
                conf_sub_estado = ACEPTAR;
                conf_aceptar = 1;
                tmr0_iniciar_6s();
            } else { /* ACEPTAR */
                if (conf_aceptar) {
                    reloj.alarma_horas   = conf_horas_tmp;
                    reloj.alarma_minutos = conf_minutos_tmp;
                    reloj.b_alarma       = 1;
                }
                reloj.modo = VISUALIZAR_HORA;
                iniciar_visualizacion();
            }
        }
    }
}

/* ══════════════════════════════════════════════════════════════════════════════
 *  Tick de reloj — llamar desde la ISR de CCP1 cada 10 ms
 * ══════════════════════════════════════════════════════════════════════════════ */
void reloj_tick(void) {
    if (reloj.centesimas >= 99) {
        reloj.centesimas = 0;
        if (reloj.segundos >= 59) {
            reloj.segundos = 0;
            if (reloj.minutos >= 59) {
                reloj.minutos = 0;
                reloj.horas = (reloj.horas >= 23) ? 0 : reloj.horas + 1;
            } else {
                reloj.minutos++;
            }
            /* Verificar alarma al inicio de cada minuto nuevo */
            if (reloj.b_alarma && !reloj.b_alarma_sonando) {
                if (reloj.horas   == reloj.alarma_horas &&
                    reloj.minutos == reloj.alarma_minutos) {
                    reloj.b_alarma_sonando = 1;
                }
            }
        } else {
            reloj.segundos++;
        }
    } else {
        reloj.centesimas++;
    }
}


void iniciar_reloj(void) {
    /* E/S */
    TRISB  = 0x07;                  /* RB0-2 como entradas               */
    ANSELB = 0xF8;                  /* RB0-2 como digitales              */
    WPUB   = 0x07;                  /* pull-ups en RB0-2                 */
    /* TMR1 + CCP1 — base de tiempo 10 ms (centésimas) */
    TMR1CLK = 0x01;                 /* TMR1 source FOSC/4                */
    T1CON   = 0x33;                 /* TMR1 ON, PSC 1:8, RD16=1         */
    CCP1CON = 0x8B;                 /* CCP1 ON, compare mode rst TMR1    */
    CCPR1H  = 0x27;
    CCPR1L  = 0x10;                 // valor de comparación 10000        
    // TMR0 — medición de pulsación - timeout (16-bit, FOSC/4, PSC 1:1024) 
    T0CON0 = 0x10;                  // 16-bit, OFF, postscaler 1:1       
    T0CON1 = 0x4A;                  // FOSC/4, sync, PSC 1:1024          
    /* Interrupciones */
    IPR6bits.CCP1IP = 1;            // CCP1 = alta prioridad
    IPR0bits.IOCIP  = 0;            // IOC  = baja prioridad
    IPR0bits.TMR0IP = 0;            // TMR0 = baja prioridad
    PIE0bits.IOCIE  = 1;            // Interrupcion por cambio (IOC) activado
    PIE0bits.TMR0IE = 1;            // Interrupcion por TMR0 activado
    PIR0bits.IOCIF  = 0;            // Limpiar flag de IOC
    PIR0bits.TMR0IF = 0;            // Limpiar flag de TMR0
    IOCBN = 0x07;                   // Interrupcion por flanco de bajada en RB0-2
    PIE6bits.CCP1IE = 1;            // Interrupcion por CCP1 activada
    PIR6bits.CCP1IF = 0;            // Limpiar flag de CCP1
    INTCON = 0xE0;                  // GIE, GIEL, IPEN activados

    LCD_INIT();
    iniciar_visualizacion();
}
void correr_reloj(void) {
    /* Si la ISR pidió reiniciar pantalla (timeout 6 s) */
    if (reloj.b_reiniciar_pantalla) {
        reloj.b_reiniciar_pantalla = 0;
        iniciar_visualizacion();
    }

    switch (reloj.modo) {
        case VISUALIZAR_HORA:
            visualizar_hora();
            break;
        case CONFIGURAR_ALARMA:
            visualizar_conf_alarma();
            break;
        default:
            reloj.modo = VISUALIZAR_HORA;
            break;
    }
}

/* ── ISR alta prioridad — tick de centésimas ───────────────────────────────── */
void __interrupt(high_priority) CCP1_ISR(void) {
    if (PIR6bits.CCP1IF) {
        PIR6bits.CCP1IF = 0;
        reloj_tick();
    }
}

/* ── ISR baja prioridad — botones (IOC) y TMR0 ─────────────────────────────── */
void __interrupt(low_priority) IOC_ISR(void) {

    /* ── IOC: se presionó un botón ──────────────────────────────────────── */
    if (PIR0bits.IOCIF) {

        /* MODE / Aceptar (RB1) */
        if (IOCBFbits.IOCBF1) {
            IOCBFbits.IOCBF1 = 0;
            reloj.b_cambiando_modo = 1;
            tmr0_iniciar_2s();
        }

        /* UP (RB0) */
        if (IOCBFbits.IOCBF0) {
            IOCBFbits.IOCBF0 = 0;
            if (reloj.modo == CONFIGURAR_ALARMA) {
                if (!reloj.b_cambiando_modo)
                    tmr0_iniciar_6s();
                if (conf_sub_estado == CONF_HORAS)
                    conf_horas_tmp = (conf_horas_tmp + 1) % 24;
                else if (conf_sub_estado == CONF_MINUTOS)
                    conf_minutos_tmp = (conf_minutos_tmp + 1) % 60;
                else
                    conf_aceptar = 1;
            } else {
                if (reloj.b_alarma_sonando)
                    reloj.b_alarma_sonando = 0;
            }
        }

        /* DOWN (RB2) */
        if (IOCBFbits.IOCBF2) {
            IOCBFbits.IOCBF2 = 0;
            if (reloj.modo == CONFIGURAR_ALARMA) {
                if (!reloj.b_cambiando_modo)
                    tmr0_iniciar_6s();
                if (conf_sub_estado == CONF_HORAS)
                    conf_horas_tmp = (conf_horas_tmp == 0) ? 23 : conf_horas_tmp - 1;
                else if (conf_sub_estado == CONF_MINUTOS)
                    conf_minutos_tmp = (conf_minutos_tmp == 0) ? 59 : conf_minutos_tmp - 1;
                else
                    conf_aceptar = 0;
            } else {
                if (reloj.b_alarma_sonando)
                    reloj.b_alarma_sonando = 0;
            }
        }
    }

    /* ── TMR0: desbordamiento ───────────────────────────────────────────── */
    if (PIR0bits.TMR0IF) {
        PIR0bits.TMR0IF = 0;
        if (reloj.b_cambiando_modo) {
            /* Pulsación larga detectada (≥ 2 s) */
            reloj.b_pulsacion_larga = 1;
            tmr0_detener();
        } else {
            /* Timeout 6 s en CONFIGURAR_ALARMA → volver a VISUALIZAR_HORA */
            tmr0_detener();
            reloj.modo = VISUALIZAR_HORA;
            reloj.b_reiniciar_pantalla = 1;  /* el main loop redibuja */
        }
    }
}