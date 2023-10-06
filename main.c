#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

#define PinEntrada  (P10_3)

/* Función de inicialización de un solo canal */
void adc_init(void);

/* Función para leer el voltaje de entrada desde el channel 0 */
void adc_process(void);

/*******************************************************************************
* Variables Globales
*******************************************************************************/
/* Objeto ADC */
cyhal_adc_t adc_obj;

/* Objeto del Canal 0 del ADC */
cyhal_adc_channel_t adc_chan_0_obj;

/* Configuración ADC predeterminada */
const cyhal_adc_config_t adc_config = {
        .continuous_scanning=false, // Desactivar el escaneo continuo
        .average_count=1,           // Desactivar el recuento promedio
        .vref=CYHAL_ADC_REF_VDDA,   // VREF para el canal de un solo extremo se establece en VDDA
        .vneg=CYHAL_ADC_VNEG_VSSA,  // VNEG para el canal de un solo extremo se establece en VSSA
        .resolution = 12u,          // Resolución de 12 bits
        .ext_vref = NC,             // Sin conexión
        .bypass_pin = NC };       // Sin conexión

/*******************************************************************************
* Función Principal: main
********************************************************************************
* Resumen:
* Esta es la función principal para la CPU CM4. Realiza...
*    1. Configurar e inicializar el ADC.
*    2. Cada 200 ms, lee el voltaje de entrada y muestra el voltaje de entrada en UART.
*
* Parámetros:
*  Ninguno
*
* Retorno:
*  int
*
*******************************************************************************/
int main(void)
{
    /* Variable para capturar el valor de retorno de las funciones */
    cy_rslt_t result;

#if defined(CY_DEVICE_SECURE)
    cyhal_wdt_t wdt_obj;
    /* Limpiar el temporizador del watchdog para que no provoque un reinicio */
    result = cyhal_wdt_init(&wdt_obj, cyhal_wdt_get_max_timeout_ms());
    CY_ASSERT(CY_RSLT_SUCCESS == result);
    cyhal_wdt_free(&wdt_obj);
#endif

    /* Inicializar los periféricos del dispositivo y de la placa */
    result = cybsp_init();

    /* Habilitar interrupciones globales */
    __enable_irq();

    /* Inicializar retarget-io para usar el puerto UART de depuración */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                                 CY_RETARGET_IO_BAUDRATE);

    /* Inicializar el Canal 0 */
    adc_init();

    /* Actualizar la configuración del ADC */
    result = cyhal_adc_configure(&adc_obj, &adc_config);

    for (int i = 0; i < 10; i++)
    {

        /* Muestrear el voltaje de entrada en el Canal 0 */
        adc_process();

        /* Retardo de 200 ms entre escaneos */
        cyhal_system_delay_ms(2000);
    }
}

/*******************************************************************************
 * Función Nombre: adc_init
 *******************************************************************************
 *
 * Resumen:
 * Función de inicialización de un solo canal del ADC. Esta función inicializa y
 * configura el Canal 0 del ADC.
 *
 * Parámetros:
 * Ninguno
 *
 * Retorno:
 * Ninguno
 *
 *******************************************************************************/
void adc_init(void)
{
    /* Variable para capturar el valor de retorno de las funciones */
    cy_rslt_t result;

    /* Inicializar el ADC. Se selecciona el bloque ADC que puede conectarse al pin de entrada del Canal 0 */
    result = cyhal_adc_init(&adc_obj, PinEntrada, NULL);

    /* Configuración del canal ADC */
    const cyhal_adc_channel_config_t channel_config = {
            .enable_averaging = false,  // Desactivar el promedio para el canal
            .min_acquisition_ns = 1000, // Tiempo de adquisición mínimo establecido en 1 us
            .enabled = true };          // Muestrear este canal cuando el ADC realice un escaneo

    /* Inicializar el Canal 0 y configurarlo para escanear el pin de entrada del Canal 0 en modo de un solo extremo. */
    result  = cyhal_adc_channel_init_diff(&adc_chan_0_obj, &adc_obj, PinEntrada,
                                          CYHAL_ADC_VNEG, &channel_config);

    printf("Proporcione el voltaje de entrada en el pin de entrada. \r\n\n");
}

/*******************************************************************************
 * Función Nombre: adc_process
 *******************************************************************************
 *
 * Resumen:
 * Función de proceso de un solo canal del ADC. Esta función lee el voltaje de entrada
 * y muestra el voltaje de entrada en UART.
 *
 * Parámetros:
 * Ninguno
 *
 * Retorno:
 * Ninguno
 *
 *******************************************************************************/
void adc_process(void)
{
	/* Variable para almacenar el resultado de conversión ADC del Canal 0 */
	    int32_t adc_result_0 = 0;

	    /* Leer el voltaje de entrada en microvoltios */
	    adc_result_0 = cyhal_adc_read_uv(&adc_chan_0_obj);

	    /* Convertir el valor a voltios */
	    float voltage_in_volts = (float)adc_result_0 / 1000000.0;

	    /* Imprimir voltaje de entrada en voltios */
	    printf("Entrada del Canal 0: %.2fV\r\n", voltage_in_volts);
}

