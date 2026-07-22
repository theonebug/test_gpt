#include "tiristor.h"
#include "sync.h"
#include "device.h"

typedef struct
{
    TiristorState_t State;

    TiristorMode_t Mode;

    uint8_t Active;

    uint32_t DelayUs;

    uint32_t PulseWidthUs;

} TiristorControl_t;

static TiristorControl_t Tiristor;

void Tiristor_Init(void)
{
    Tiristor.State = TIRISTOR_IDLE;

    Tiristor.Mode = TIRISTOR_MODE_FIRST;

    Tiristor.Active = 0;

    Tiristor.PulseWidthUs = 200;
}

void Tiristor_Start(void)
{
    Tiristor.Active = 1;

    Tiristor.State = TIRISTOR_WAIT_FIRST;
}

void Tiristor_Stop(void)
{
    Tiristor.Active = 0;

    Tiristor.State = TIRISTOR_IDLE;
}

uint8_t Tiristor_IsActive(void)
{
    return Tiristor.Active;
}

void Tiristor_SetMode(TiristorMode_t mode)
{
    Tiristor.Mode = mode;
}

TiristorMode_t Tiristor_GetMode(void)
{
    return Tiristor.Mode;
}

void Tiristor_OnZeroCross(void)
{
    if(Tiristor.Active == 0)
    {
        return;
    }

    switch(Tiristor.State)
    {
        case TIRISTOR_WAIT_FIRST:



            Tiristor.State = TIRISTOR_PULSE;

            break;

        case TIRISTOR_CONDUCTION:

            if(Tiristor.Mode == TIRISTOR_MODE_ALL)
            {


            	Tiristor.State = TIRISTOR_PULSE;
            }

            break;

        default:

            break;
    }
}

uint32_t Tiristor_GetDelay(void)
{
    return Tiristor.DelayUs;
}

void Tiristor_Update(void)
{
    switch(Tiristor.State)
    {
        case TIRISTOR_IDLE:

            break;

        case TIRISTOR_WAIT_FIRST:

            break;

        case TIRISTOR_PULSE:

        	Tiristor.DelayUs =
        	    (SYNC_GetHalfPeriodUs() * Device_GetAngle()) / 180;

            Tiristor.State = TIRISTOR_CONDUCTION;

            break;

        case TIRISTOR_CONDUCTION:

            /*
             * Ждем следующий переход через ноль
             */

            break;

        case TIRISTOR_FINISHED:

            Tiristor_Stop();

            break;

        default:

            Tiristor.State = TIRISTOR_IDLE;

            break;
    }
}

