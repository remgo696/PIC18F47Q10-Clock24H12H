#include <xc.h>
#include "main.h"
#include "reloj.h"

/* ── Configuración del hardware ────────────────────────────────────────────── */
static void configuro(void) {
    /* Oscilador */
    OSCCON1 = 0x60;
    OSCFRQ  = 0x06;                 /* HFINTOSC a 32 MHz                 */
    OSCEN   = 0x40;
    /* RELOJ */
    iniciar_reloj();
}

/* ── Main ──────────────────────────────────────────────────────────────────── */
void main(void) {
    configuro();

    while (1) {
        correr_reloj();
    }
}