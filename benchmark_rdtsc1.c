/**
 * @file benchmark_rdtsc1.c
 * @author desmonHak
 * @brief 
 * @version 0.1
 * @date 2025-04-15
 * 
 * @copyright Copyright (c) 2025
 * 
 * Estrategias para Robustecer las Mediciones
 *
 *  Invalidate (Vaciar) la Caché Antes de Medir:
 *  Puedes forzar la invalidación de las líneas de caché que puedan interferir c
 * on la medición usando la instrucción _mm_clflush(). Esta instrucción permite
 * “limpiar” (invalidar) una línea de caché de una dirección de memoria 
 * determinada. Por ejemplo, si mides un fragmento de código que accede a 
 * ciertos datos, asegúrate de vaciar esas líneas de caché.
 *
 *  Realizar Múltiples Mediciones y Promediar:
 *  En lugar de medir una única ejecución, ejecuta la prueba varias veces y 
 * calcula el promedio. Esto reduce el efecto de fluctuaciones debidas a 
 * interferencias del sistema o cambios en el estado de la caché.
 *
 *  Utilizar Instrucciones de Serialización (CPUID, Memory Fences):
 *  Ya usas __cpuid para asegurarte de que las instrucciones anteriores se 
 * hayan completado. Puedes incluso incluir instrucciones de tipo mfence 
 * (o _mm_mfence()) para reforzar las barreras de memoria si es necesario.
 *
 *  Aislar el Código Medido del Código de Preparación:
 *  Evita que la preparación de datos (que pueda provocar la carga de la 
 * caché) se incluya dentro del tiempo medido. Por ello, separa claramente 
 * la fase de “warmup” o inicialización de la fase de medición.
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <emmintrin.h>  // Para _mm_clflush

#ifdef _MSC_VER
    #include <intrin.h>
    #pragma intrinsic(__rdtsc)
#else
    #include <x86intrin.h>
#endif

// Variables globales de configuración
uint64_t g_correctionFactor;  // Número de ciclos para compensar la sobrecarga del medidor
uint64_t g_accuracy;          // Precisión (variabilidad) de la medición

// Constante para marcar mediciones inválidas (no utilizada en este ejemplo)
static const uint64_t errormeasure = ~((uint64_t)0);

// Función que retorna el contador de ciclos actual (RDTSC)
uint64_t GetRDTSC(void) {
    unsigned int a[4];
#ifdef _MSC_VER
    __cpuid((int*)a, 0x80000000);  // Forzar serialización
    return __rdtsc();
#else
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
    // Para otros compiladores
#endif

    uint64_t minDiff = UINT64_MAX;  
    uint64_t maxDiff = 0;           

    // Se realizan 80 mediciones consecutivas para obtener la sobrecarga mínima y máxima
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

// Función para vaciar la caché
// Se asigna y se recorre un bloque de memoria mayor al tamaño de la última caché
void flushCache(void) {
    const size_t cacheSize = 10 * 1024 * 1024; // 10 MB, ajusta según la arquitectura
    char *buffer = (char*)malloc(cacheSize);
    if (!buffer) {
        perror("No se pudo asignar el buffer para flushCache");
        return;
    }
    // Forzar la carga y luego invalidar las líneas de caché
    for (size_t i = 0; i < cacheSize; i += 64) {
        buffer[i] = (char)i;
        _mm_clflush(&buffer[i]);
    }
    // Opcional: un barrier para asegurar que se hayan aplicado las operaciones
#ifdef _MSC_VER
    _mm_mfence();
#else
    __builtin_ia32_mfence();
#endif
    free(buffer);
}

// Función que realiza la operación a medir. En este ejemplo, un bucle que suma números.
volatile int dummy;  // Declarado globalmente para evitar optimizaciones
void testOperation(void) {
    dummy = 0;
    for (int i = 0; i < 100000000; i++) {
        dummy += i;
    }
}

int main(void) {
    printf("Calibrando el medidor RDTSC...\n");
    WarmupRDTSC();

    const int iterations = 20; // Número de mediciones para promediar
    uint64_t totalElapsed = 0;

    for (int i = 0; i < iterations; i++) {
        // Vaciar caché para evitar que datos de iteraciones previas influyan
        flushCache();

        // Serializar instrucciones antes de la medición (opcional)
#ifdef _MSC_VER
        int cpuidDummy[4];
        __cpuid((int*)cpuidDummy, 0x80000000);
#else
        unsigned int dummyArray[4];
        int val = 0x80000000;
        __cpuid(&val, dummyArray[0], dummyArray[1], dummyArray[2], dummyArray[3]);
#endif

        uint64_t start = GetRDTSC();
        testOperation();
        uint64_t end = GetRDTSC();

        uint64_t elapsed = (end - start);
        if (elapsed > g_correctionFactor) {
            elapsed -= g_correctionFactor;
        }

        printf("Iteracion %d - Elapsed clocks: %llu\n", i + 1, elapsed);
        totalElapsed += elapsed;
    }

    uint64_t averageElapsed = totalElapsed / iterations;
    printf("Tiempo promedio (en ciclos) para testOperation: %llu\n", averageElapsed);

    return 0;
}
