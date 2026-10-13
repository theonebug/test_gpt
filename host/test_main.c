/*
 * test_main.c - хост-тест логики режима FIRST WAVE / PassThrough.
 *
 * Собирает оригинальные модули прошивки (Core/Device/device.c,
 * Core/Device/tiristor.c) вместе с хост-заглушками периферии
 * (host/sim) и проверяет, что:
 *
 *   - в режиме FIRST WAVE первая полуволна обрезается по заданному углу,
 *     а последующие открываются через настраиваемую задержку
 *     PassMinDelayUs (+ ZeroCrossOffsetUs) после перехода через ноль;
 *   - импульс формируется на КАЖДОЙ полуволне (CC1IE не гасится);
 *   - в режимах CONTINUOUS и TIMED срез по углу выполняется на КАЖДОЙ
 *     полуволне, задержка монотонно зависит от угла (регулирование не
 *     теряется).
 *
 * Между переходами через ноль реально вызываются обработчики каналов
 * Tiristor_Channel1_IRQHandler / Tiristor_Channel2_IRQHandler
 * (через HOST_Mock_FireChannel1/2), как на настоящем железе.
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
void HOST_Mock_FireChannel1(void);
void HOST_Mock_FireChannel2(void);
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

/*--- Результаты пробега: CCR1 и факт формирования импульса ---*/
static uint32_t Ccr1Log[16];
static uint8_t  PulseLog[16];   /* 1 - выход поднимался и гасился на волне */

/* Вспомогательное: перейти устройство в состояние TEST и отдать N волн.
   Между волнами вызываются реальные обработчики каналов TIM2. */
static void run_halfwaves(WorkMode_t mode, uint16_t angle,
                          uint16_t pulseUs, uint16_t zcOffsetUs,
                          uint16_t passMinUs,
                          int waves)
{
    int i;

    /* Инициализация прибора и тиристора */
    Device_Init();
    Tiristor_Init();

    Settings.WorkMode          = mode;
    Settings.PulseWidthUs      = pulseUs;
    Settings.ZeroCrossOffsetUs = zcOffsetUs;
    Settings.PassMinDelayUs    = passMinUs;
    Settings.DurationMs        = 1000U;
    Settings.MaxRunTimeS       = 10U;

    Device_SetAngle(angle);

    /* Применить параметры так, как это делает SETTINGS_Apply() */
    Tiristor_SetPulseWidthUs(Settings.PulseWidthUs);
    Tiristor_SetZeroCrossOffsetUs(Settings.ZeroCrossOffsetUs);
    Tiristor_SetPassMinDelayUs(Settings.PassMinDelayUs);
    Tiristor_SetMode(TIRISTOR_MODE_ALL);

    /* Синхронизация присутствует, полупериод 10 мс (50 Гц) */
    HOST_SYNC_SetPresent(1U);
    HOST_SYNC_SetHalfPeriodUs(10000U);
    HOST_Mock_SetTick(0U);

    /* READY -> WAIT_SYNC (нужна синхронизация) */
    Device_Start();
    /* WAIT_SYNC -> WAIT_ZERO (первый тик главного цикла) */
    Device_Update();
    /* Первая полуволна переводит WAIT_ZERO -> TEST и начинает работу */

    for(i = 0; i < waves; i++)
    {
        Device_OnZeroCross();
        Ccr1Log[i] = TIM2_.CCR1;

        /* Такты таймера: срабатывание сравнений CH1 (открытие) и
           CH2 (гашение выхода) - реальные обработчики тиристора. */
        HOST_Mock_FireChannel1();
        {
            uint8_t high = ((GPIOC->ODR & TIRISTOR_OUT_Pin) != 0U) ? 1U : 0U;

            HOST_Mock_FireChannel2();

            PulseLog[i] = (high &&
                           ((GPIOC->ODR & TIRISTOR_OUT_Pin) == 0U)) ? 1U : 0U;
        }

        HOST_Mock_AdvanceTick(20U);
    }
}

/* Задержка по углу (как в Tiristor_OnZeroCross), для сверки ожидаемого */
static uint32_t expected_angle_delay(uint16_t angle, uint16_t pulseUs,
                                     uint16_t zcOffsetUs)
{
    uint32_t halfPeriod = 10000U;
    uint32_t delay = (uint32_t)zcOffsetUs + ((halfPeriod * angle) / 180U);
    uint32_t max_delay = halfPeriod - pulseUs - 50U; /* GUARD = 50 */
    if(delay < 10U) delay = 10U;    /* TIRISTOR_MIN_DELAY_US */
    if(delay > max_delay) delay = max_delay;
    return delay;
}

/* Задержка PassThrough (как в Tiristor_OnZeroCross при PassThrough=1) */
static uint32_t expected_pass_delay(uint16_t zcOffsetUs, uint16_t passMinUs)
{
    uint32_t halfPeriod = 10000U;
    uint16_t pulseUs = 100U;
    uint32_t delay = (uint32_t)zcOffsetUs + passMinUs;
    uint32_t max_delay = halfPeriod - pulseUs - 50U;
    if(delay < 10U) delay = 10U;
    if(delay > max_delay) delay = max_delay;
    return delay;
}

int main(void)
{
    printf("=== HOST-TEST: FIRST WAVE / PassThrough / PassMinDelayUs ===\n\n");

    uint16_t angle = 90U;
    uint16_t pulse = 100U;
    uint16_t zc    = 0U;
    uint16_t passMin = 50U;         /* дефолт PassMinDelayUs */
    uint32_t expectAngle = expected_angle_delay(angle, pulse, zc);
    uint32_t expectPass  = expected_pass_delay(zc, passMin);
    int N = 5;
    int i;
    char m[112];

    printf("[1] FIRST WAVE: 1-я волна по углу, далее PassMinDelayUs; "
           "импульс на КАЖДОЙ волне\n");
    run_halfwaves(WORK_MODE_FIRST_WAVE, angle, pulse, zc, passMin, N);
    for(i = 0; i < N; i++)
    {
        uint32_t expect = (i == 0) ? expectAngle : expectPass;

        snprintf(m, sizeof m, "волна %d: CCR1==%lu (ожидалось %lu)",
                 i+1, (unsigned long)Ccr1Log[i], (unsigned long)expect);
        CHECK(Ccr1Log[i] == expect, m);

        snprintf(m, sizeof m,
                 "волна %d: импульс сформирован (выход поднят и погашен)", i+1);
        CHECK(PulseLog[i] == 1U, m);
    }

    /* Регресс A1: CC1IE не должен гаситься после волн */
    snprintf(m, sizeof m, "CC1IE остался разрешён после %d волн", N);
    CHECK((TIM2_.DIER & TIM_DIER_CC1IE) != 0U, m);

    printf("\n[2] FIRST WAVE: калибровка нуля + ненулевой PassMinDelayUs\n");
    zc = 100U;
    passMin = 200U;
    expectAngle = expected_angle_delay(angle, pulse, zc);
    expectPass  = expected_pass_delay(zc, passMin);
    run_halfwaves(WORK_MODE_FIRST_WAVE, angle, pulse, zc, passMin, N);
    for(i = 0; i < N; i++)
    {
        uint32_t expect = (i == 0) ? expectAngle : expectPass;

        snprintf(m, sizeof m, "волна %d: CCR1==%lu (ожидалось %lu)",
                 i+1, (unsigned long)Ccr1Log[i], (unsigned long)expect);
        CHECK(Ccr1Log[i] == expect, m);
    }
    zc = 0U;
    passMin = 50U;
    expectAngle = expected_angle_delay(angle, pulse, zc);

    printf("\n[3] CONTINUOUS: срез по углу на каждой волне\n");
    run_halfwaves(WORK_MODE_CONTINUOUS, angle, pulse, zc, passMin, N);
    for(i = 0; i < N; i++)
    {
        snprintf(m, sizeof m, "волна %d: CCR1==%lu (по углу)",
                 i+1, (unsigned long)Ccr1Log[i]);
        CHECK(Ccr1Log[i] == expectAngle, m);

        snprintf(m, sizeof m, "волна %d: импульс сформирован", i+1);
        CHECK(PulseLog[i] == 1U, m);
    }

    printf("\n[4] TIMED: срез по углу на каждой волне\n");
    run_halfwaves(WORK_MODE_TIMED, angle, pulse, zc, passMin, N);
    for(i = 0; i < N; i++)
    {
        snprintf(m, sizeof m, "волна %d: CCR1==%lu (по углу)",
                 i+1, (unsigned long)Ccr1Log[i]);
        CHECK(Ccr1Log[i] == expectAngle, m);
    }

    printf("\n[5] CONTINUOUS: монотонность регулирования по углу 5...175\n");
    {
        const uint16_t angles[] = { 5U, 45U, 90U, 135U, 175U };
        uint32_t prev = 0U;
        int k;
        int mono_ok = 1;

        for(k = 0; k < (int)(sizeof(angles)/sizeof(angles[0])); k++)
        {
            uint32_t expect = expected_angle_delay(angles[k], pulse, zc);

            run_halfwaves(WORK_MODE_CONTINUOUS, angles[k], pulse, zc,
                          passMin, 1);

            snprintf(m, sizeof m, "угол %u град: CCR1==%lu (ожидалось %lu)",
                     angles[k], (unsigned long)Ccr1Log[0],
                     (unsigned long)expect);
            CHECK(Ccr1Log[0] == expect, m);

            if(k > 0 && Ccr1Log[0] <= prev) { mono_ok = 0; }
            prev = Ccr1Log[0];
        }

        CHECK(mono_ok, "задержка строго растёт с углом (регулирование не потеряно)");
    }

    printf("\n=== ИТОГ: %s ===\n", failures == 0 ? "ВСЕ ТЕСТЫ ПРОЙДЕНЫ" : "ЕСТЬ ПРОВАЛЫ");
    return failures;
}
