/*
 * test_main.c - хост-тест логики режима FIRST WAVE / PassThrough.
 *
 * Собирает оригинальные модули прошивки (Core/Device/device.c,
 * Core/Device/tiristor.c) вместе с хост-заглушками периферии
 * (host/sim) и проверяет, что:
 *
 *   - в режиме FIRST WAVE первая полуволна обрезается по заданному углу,
 *     а последующие открываются сразу после перехода через ноль
 *     (PassThrough: минимальная задержка);
 *   - в режимах CONTINUOUS и TIMED срез по углу выполняется на КАЖДОЙ
 *     полуволне (прежнее поведение не меняется).
 *
 * Возвращает 0 при успехе, иначе число провалов (1..N).
 */

#include <stdio.h>

#include "device.h"
#include "tiristor.h"
#include "settings.h"
#include "sync.h"
#include "tim.h"
#include "main.h"
#include "bsp_lcd.h"

/* Settings определяется здесь (не тянем settings.c - там Flash/Modbus) */
Settings_t Settings;

/*--- Хостовые управляющие функции (из hal_mock.c / sync_stub.c) ---*/
void HOST_Mock_SetTick(uint32_t t);
void HOST_Mock_AdvanceTick(uint32_t d);
void HOST_SYNC_SetPresent(uint8_t p);
void HOST_SYNC_SetHalfPeriodUs(uint32_t hp);

/*--- Счётчик тестов ---*/
static int failures = 0;

#define CHECK(cond, msg)                                    \
    do {                                                    \
        if(!(cond)) {                                       \
            failures++;                                     \
            printf("  FAIL: %s (line %d)\n", msg, __LINE__);\
        } else {                                            \
            printf("  ok: %s\n", msg);                      \
        }                                                   \
    } while(0)

/* Вспомогательное: перейти устройство в состояние TEST и отдать N волн,
   возвращает массив CCR1 для каждой волны. */
static void run_halfwaves(WorkMode_t mode, uint16_t angle,
                          uint16_t pulseUs, uint16_t zcOffsetUs,
                          int waves, uint32_t out_ccr1[])
{
    /* Инициализация прибора и тиристора */
    Device_Init();
    Tiristor_Init();

    Settings.WorkMode        = mode;
    Settings.PulseWidthUs    = pulseUs;
    Settings.ZeroCrossOffsetUs = zcOffsetUs;
    Settings.DurationMs      = 1000U;
    Settings.MaxRunTimeS     = 10U;

    Device_SetAngle(angle);

    /* Установить режим тиристора так, как это делает SETTINGS_Apply() */
    Tiristor_SetMode((Settings.WorkMode == WORK_MODE_FIRST_WAVE)
                     ? TIRISTOR_MODE_FIRST
                     : TIRISTOR_MODE_ALL);

    /* Синхронизация присутствует, полупериод 10 мс (50 Гц) */
    HOST_SYNC_SetPresent(1U);
    HOST_SYNC_SetHalfPeriodUs(10000U);
    HOST_Mock_SetTick(0U);

    /* READY -> WAIT_SYNC (нужна синхронизация) */
    Device_Start();
    /* WAIT_SYNC -> WAIT_ZERO (первый тик главного цикла) */
    Device_Update();
    /* Первая полуволна переводит WAIT_ZERO -> TEST и начинает работу */

    int i;
    for(i = 0; i < waves; i++)
    {
        Device_OnZeroCross();
        out_ccr1[i] = TIM2_.CCR1;
        HOST_Mock_AdvanceTick(20U);
    }
}

/* Задержка по углу (как в Tiristor_OnZeroCross), для сверки ожидаемого */
static uint32_t expected_angle_delay(uint16_t angle, uint16_t pulseUs,
                                     uint16_t zcOffsetUs)
{
    uint32_t halfPeriod = 10000U;
    uint32_t delay = zcOffsetUs + ((halfPeriod * angle) / 180U);
    uint32_t max_delay = halfPeriod - pulseUs - 50U; /* GUARD = 50 */
    if(delay < 10U) delay = 10U;    /* TIRISTOR_MIN_DELAY_US */
    if(delay > max_delay) delay = max_delay;
    return delay;
}

int main(void)
{
    printf("=== HOST-TEST: FIRST WAVE / PassThrough ===\n\n");

    /* Угол 90° => задержка 5000 мкс (полупериод 10000 мкс) */
    uint16_t angle = 90U;
    uint16_t pulse = 100U;
    uint16_t zc    = 0U;
    uint32_t expectAngle = expected_angle_delay(angle, pulse, zc);
    uint32_t expectPass  = 10U;   /* TIRISTOR_MIN_DELAY_US */
    int N = 5;
    uint32_t ccr1[8];

    printf("[1] FIRST WAVE: 1-я волна по углу, последующие сразу после нуля\n");
    run_halfwaves(WORK_MODE_FIRST_WAVE, angle, pulse, zc, N, ccr1);
    {
        int i;
        for(i = 0; i < N; i++)
        {
            if(i == 0)
            {
                char m[96];
                snprintf(m, sizeof m, "волна %d: CCR1==%lu (по углу)",
                         i+1, (unsigned long)expectAngle);
                CHECK(ccr1[i] == expectAngle, m);
            }
            else
            {
                char m[96];
                snprintf(m, sizeof m, "волна %d: CCR1==%lu (PassThrough, сразу после нуля)",
                         i+1, (unsigned long)expectPass);
                CHECK(ccr1[i] == expectPass, m);
            }
        }
    }

    printf("\n[2] CONTINUOUS: срез по углу на каждой волне (поведение прежнее)\n");
    run_halfwaves(WORK_MODE_CONTINUOUS, angle, pulse, zc, N, ccr1);
    {
        int i;
        for(i = 0; i < N; i++)
        {
            char m[64];
            snprintf(m, sizeof m, "волна %d: CCR1==%lu (по углу)",
                     i+1, (unsigned long)expectAngle);
            CHECK(ccr1[i] == expectAngle, m);
        }
    }

    printf("\n[3] TIMED: срез по углу на каждой волне (поведение прежнее)\n");
    run_halfwaves(WORK_MODE_TIMED, angle, pulse, zc, N, ccr1);
    {
        int i;
        for(i = 0; i < N; i++)
        {
            char m[64];
            snprintf(m, sizeof m, "волна %d: CCR1==%lu (по углу)",
                     i+1, (unsigned long)expectAngle);
            CHECK(ccr1[i] == expectAngle, m);
        }
    }

    printf("\n=== ИТОГ: %s ===\n", failures == 0 ? "ВСЕ ТЕСТЫ ПРОЙДЕНЫ" : "ЕСТЬ ПРОВАЛЫ");
    return failures;
}