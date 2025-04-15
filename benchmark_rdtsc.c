/**
 * @file benchmark_rdtsc.c
 * @author desmonHak
 * @brief 
 * @version 0.1
 * @date 2025-04-15
 * 
 * @copyright Copyright (c) 2025
 * 
 * 1. Corrección de la Sobrecarga
 *     
 *     Factor de corrección (g_correctionFactor):
 *     Este valor representa el menor número de ciclos que tarda la propia 
 *     llamada a GetRDTSC(). En tus ejecuciones se muestran valores de 1084, 
 *     1716 y 3640 ciclos. Esto es lo esperado porque, aunque puede variar 
 *     ligeramente debido al estado del procesador o interferencias del sistema, 
 *     este número debe ser significativamente menor que el tiempo total medido
 *  de las operaciones a cronometrar.
 * 
 *         Verificación:
 *         Si el factor de corrección es extremadamente alto o comparable al 
 *         tiempo medido de la operación (en este caso, el bucle), ello indicaría 
 *         que la sobrecarga del medidor domina la medición, lo cual sería una 
 *         señal de problema. En tu ejemplo, los números están en el rango de 
 *         miles mientras que la ejecución del bucle está en el rango de 
 *         cientos de millones de ciclos, lo que es correcto.
 * 
 * 2. Precisión de la Medición
 * 
 *     Accuracy (g_accuracy):
 *     Este valor, calculado como la diferencia entre la medición máxima y la 
 *     mínima en las lecturas consecutivas, da una idea de la variabilidad del 
 *     instrumento de medición. En tus ejemplos aparecen valores tales como 
 *     47563, 630 y 990 ciclos.
 * 
 *         Verificación:
 *         Si la diferencia fuera muy grande de forma sistemática, podría 
 *         indicar fluctuaciones o interferencias externas (por ejemplo, 
 *         interrupciones del sistema, cambios en la planificación de 
 *         procesos, etc.) que afectan la lectura. En tus resultados, algunas 
 *         ejecuciones muestran una variabilidad baja (630 o 990 ciclos) y 
 *         otra más alta (47563 ciclos); esto puede depender de condiciones 
 *         momentáneas, pero en términos generales, si la variabilidad es baja
 *         o moderada frente a una gran cantidad de ciclos medidos, el sistema 
 *         se considera estable.
 * 
 * 3. Consistencia en las Medidas
 * 
 *     Tiempo de Ejecución del Bucle (Elapsed clocks):
 *     Los tiempos medidos para el bucle (por ejemplo, 217743449, 215501907 y 
 *     191106480 ciclos) varían de una ejecución a otra, lo cual es normal dada 
 *     la naturaleza de las mediciones a nivel de hardware.
 * 
 *         Verificación:
 * 
 *             Las diferencias entre ejecuciones deben ser relativamente 
 *             pequeñas en comparación con el total medido.
 * 
 *             Es normal que existan ligeras variaciones por causas externas 
 *             (tareas del sistema operativo, caché, etc.).
 * 
 *             Si el patrón es consistente (por ejemplo, siempre del mismo orden 
 *             de magnitud), se puede confirmar que el medidor está funcionando como se espera.
 * 
 * 4. Comparación con Otros Medidores
 * 
 *     Si cuentas con otra herramienta de medición de tiempos de alta resolución 
 *     (por ejemplo, funciones de temporización en el sistema operativo), 
 *     comparar ambos resultados te ayudará a validar que la función RDTSC 
 *     está dando valores coherentes.
 * 
 * Conclusión
 * 
 * El código cumple su función correctamente si:
 * 
 *     El factor de corrección es bajo en comparación con el tiempo total 
 *     medido (lo cual se cumple en tus ejecuciones).
 * 
 *     La precisión (accuracy) muestra una variabilidad razonable y 
 *     constante en múltiples mediciones.
 * 
 *     Los tiempos medidos para la misma operación (en este caso, el bucle) 
 *     son consistentes en orden de magnitud, aunque admitan ligeras 
 *     variaciones por factores externos.
 * 
 * En tu ejemplo, aunque se observan pequeñas variaciones de una ejecución a 
 * otra (lo cual es normal en mediciones en tiempo real), el comportamiento 
 * general es correcto y se infiere que la herramienta de medición está 
 * funcionando de acuerdo con su propósito.
 * 
 * 
 *     Inclusión de Cabeceras y Definiciones:
 *      Se incluyen cabeceras estándar como <stdio.h>, <stdint.h>, y <limits.h>. 
 *      Para MSVC se incluye <intrin.h> y se usa #pragma intrinsic(__rdtsc); 
 *      para otros compiladores se incluye <x86intrin.h>.
 *  
 *      Variables Globales:
 *  
 *          g_correctionFactor: Guarda el número mínimo de ciclos entre dos 
 *          llamadas consecutivas a GetRDTSC(), es decir, la sobrecarga mínima 
 *          del medidor.
 *  
 *          g_accuracy: Mide la diferencia (variabilidad) entre la mayor y la 
 *          menor sobrecarga encontrada.
 *  
 *      Función GetRDTSC():
 *      Utiliza la instrucción __rdtsc() para leer el contador de ciclos del 
 *      CPU. Se invoca también __cpuid para garantizar que se vacíe el pipeline 
 *      de instrucciones antes de la lectura.
 *  
 *      Función WarmupRDTSC():
 *      Realiza varias mediciones consecutivas (80 en total) para determinar 
 *      la sobrecarga mínima y máxima. Con estos valores se establecen:
 *  
 *          g_correctionFactor: Valor mínimo de ciclos medidos.
 *  
 *          g_accuracy: Diferencia entre la medición máxima y mínima, que sirve 
 *          como un indicador de la precisión de la medición.
 *  
 *      Función main() – Ejemplo de Uso:
 *  
 *          Se ejecuta WarmupRDTSC() para calibrar el medidor.
 *  
 *          Se mide el tiempo (en ciclos de CPU) que tarda un bucle de 100 
 *          millones de iteraciones.
 *  
 *          Se compensa la sobrecarga restando g_correctionFactor y se muestra 
 *          en la salida estándar el número de ciclos consumidos.
 *  
 *  Este código en C es útil para tener una medición de alta resolución en 
 *  entornos en los que se desea conocer el rendimiento de fragmentos críticos 
 *  del código, compensando la sobrecarga de la función de medición en sí.
 * https://stackoverflow.com/questions/21166675/boostflat-map-and-its-performance-compared-to-map-and-unordered-map
 * 
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

#ifdef _MSC_VER
    #include <intrin.h>
    #pragma intrinsic(__rdtsc)
#else
    #include <x86intrin.h>
#endif

// Variables globales de configuración
uint64_t g_correctionFactor;  // Número de ciclos a restar para compensar la sobrecarga del medidor
uint64_t g_accuracy;          // Precisión (variabilidad) de la medición

// Constante de error (se puede usar para marcar mediciones inválidas)
static const uint64_t errormeasure = ~((uint64_t)0);

// Función que retorna el contador de ciclos actual (RDTSC)
uint64_t GetRDTSC(void) {
    unsigned int a[4];
#ifdef _MSC_VER
    // Se llama a __cpuid para limpiar el pipeline de instrucciones
    __cpuid((int*)a, 0x80000000);
    return __rdtsc();
#else
    // En otros compiladores se utiliza __cpuid y __rdtsc disponibles en x86intrin.h
    long val = 0x80000000;
    __cpuid(&val, a[0], a[1], a[2], a[3]);
    return __rdtsc();
#endif
}

// Función para "calentar" y calibrar la medición del contador de ciclos
void WarmupRDTSC(void) {
    unsigned int a[4];
#ifdef _MSC_VER
    __cpuid((int*)a, 0x80000000);
    __cpuid((int*)a, 0x80000000);
    __cpuid((int*)a, 0x80000000);
#else
    // En compiladores no MSVC se puede llamar a __cpuid si está implementado o simplemente omitirlo
#endif

    uint64_t minDiff = UINT64_MAX;  // Valor inicial alto para buscar el mínimo
    uint64_t maxDiff = 0;           // Valor inicial bajo para buscar el máximo

    // Se realizan 80 mediciones consecutivas para obtener una estimación de la sobrecarga
    for (int i = 0; i < 80; ++i) {
        uint64_t tick1 = GetRDTSC();
        uint64_t tick2 = GetRDTSC();
        uint64_t diff = tick2 - tick1;
        if (diff < minDiff) {
            minDiff = diff;
        }
        if (diff > maxDiff) {
            maxDiff = diff;
        }
    }
    g_correctionFactor = minDiff;
    printf("Correction factor %llu clocks\n", g_correctionFactor);

    g_accuracy = maxDiff - minDiff;
    printf("Measurement Accuracy (in clocks) : %llu\n", g_accuracy);
}

// Ejemplo de uso: medir el tiempo que tarda en ejecutarse un bucle
int main(void) {
    printf("Calibrando el medidor RDTSC...\n");
    WarmupRDTSC();
    
    // Ejemplo: medir el tiempo (en ciclos) de ejecutar un bucle que suma números
    uint64_t start = GetRDTSC();
    
    // Operación a medir: bucle que recorre 100 millones de iteraciones
    volatile int dummy = 0;  // La palabra clave volatile evita optimizaciones indeseadas
    for (int i = 0; i < 100000000; i++) {
        dummy += i;
    }
    
    uint64_t end = GetRDTSC();
    uint64_t elapsed = end - start;
    
    // Se compensa la sobrecarga de la medición (si es aplicable)
    if (elapsed > g_correctionFactor) {
        elapsed -= g_correctionFactor;
    }
    
    printf("Elapsed clocks for loop: %llu\n", elapsed);
    return 0;
}
