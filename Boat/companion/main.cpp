#include "pico/stdlib.h"
#include "pico/i2c_slave.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/watchdog.h"
#include "hardware/sync.h"
#include "carrier_safety_core.h"
#include "carrier_output_plan.h"
#include <cstring>
using namespace carrier;
// Raspberry Pi Pico physical pin mapping in R5 connector schedule.
// Deliberately NOT commissioned; no key, jumper or I2C packet changes this.
// Replace these values only through a separately reviewed hardware profile.
static constexpr bool commissioned=false;
static constexpr uint16_t pulseLimit[4]={0,0,0,0};
static constexpr uint16_t pwmLimit=0,reverseMs=0;
static constexpr bool invertLeft=false,invertRight=false;
static SafetyCore core;
static CarrierOutputPlan planner;
static uint8_t rx[SIZE],pending[SIZE],reply[SIZE];
static volatile unsigned rxLen=0,txIndex=0;
static volatile bool pendingReady=false,rxOverflow=false,busFault=false;
static uint16_t sample=0;
static constexpr uint SDA=0,SCL=1,PWM_L=2,PWM_R=3,DIR_L=4,DIR_R=5,
 COIL0=6,WDI=10,PERMIT=11,STOP_FB=12,GATE_FB=13,L_IDLE=14,R_IDLE=15,
 COILS_IDLE_FB=16,INTERLOCK_FB=17,IDLE0=18,WD_OK_FB=26,FEEDBACK_VALID=27;
static void i2cEvent(i2c_inst_t *i2c,i2c_slave_event_t event){
 switch(event){
 case I2C_SLAVE_RECEIVE:
  while(i2c_get_read_available(i2c)){
   uint8_t b=i2c_read_byte_raw(i2c);
   if(rxLen<SIZE)rx[rxLen++]=b;else rxOverflow=true;
  }
  break;
 case I2C_SLAVE_REQUEST:
  i2c_write_byte_raw(i2c,txIndex<SIZE?reply[txIndex++]:0xff);break;
 case I2C_SLAVE_FINISH:
  if(rxLen||rxOverflow){
   if(rxLen!=SIZE||rxOverflow||pendingReady)busFault=true;
   else{memcpy(pending,rx,SIZE);pendingReady=true;}
  }
  rxLen=0;txIndex=0;rxOverflow=false;break;
 }
}
static void outputLow(uint p){gpio_init(p);gpio_put(p,false);gpio_set_dir(p,GPIO_OUT);}
static void inputLow(uint p){gpio_init(p);gpio_set_dir(p,GPIO_IN);gpio_pull_down(p);}
static Inputs readInputs(){
 Inputs in{};
 if(gpio_get(STOP_FB))in.flags|=STOP_OK;
 if(gpio_get(WD_OK_FB))in.flags|=WATCHDOG_OK;
 if(gpio_get(INTERLOCK_FB))in.flags|=INTERLOCK_OK;
 if(gpio_get(GATE_FB))in.flags|=GATE_ON;
 if(gpio_get(L_IDLE))in.flags|=LEFT_IDLE;
 if(gpio_get(R_IDLE))in.flags|=RIGHT_IDLE;
 if(gpio_get(COILS_IDLE_FB))in.flags|=COILS_IDLE;
 if(gpio_get(FEEDBACK_VALID))in.flags|=FEEDBACK_OK;
 for(unsigned i=0;i<4;++i)if(gpio_get(IDLE0+i))in.coilIdleMask|=1U<<i;
 return in;
}
static void motorDuty(uint p,uint16_t v){pwm_set_gpio_level(p,v);}
static void off(){
 gpio_put(PERMIT,false);motorDuty(PWM_L,0);motorDuty(PWM_R,0);
 for(unsigned i=0;i<4;++i)gpio_put(COIL0+i,false);
}
int main(){
 outputLow(PERMIT);outputLow(WDI);outputLow(DIR_L);outputLow(DIR_R);
 for(unsigned i=0;i<4;++i)outputLow(COIL0+i);
 for(uint p=STOP_FB;p<=IDLE0+3;++p)inputLow(p);
 inputLow(WD_OK_FB);inputLow(FEEDBACK_VALID);
 outputLow(PWM_L);outputLow(PWM_R);
 // GP2/3 are the two channels of one PWM slice.
 uint slice=pwm_gpio_to_slice_num(PWM_L);pwm_config cfg=pwm_get_default_config();
 pwm_config_set_wrap(&cfg,1023);pwm_config_set_clkdiv(&cfg,clock_get_hz(clk_sys)/(20000.0f*1024));
 pwm_init(slice,&cfg,false);motorDuty(PWM_L,0);motorDuty(PWM_R,0);
 gpio_set_function(PWM_L,GPIO_FUNC_PWM);gpio_set_function(PWM_R,GPIO_FUNC_PWM);pwm_set_enabled(slice,true);
 core.configure(commissioned,pulseLimit);planner.configure(commissioned,invertLeft,invertRight,pwmLimit,reverseMs);
 encodeStatus(core.status(sample,readInputs()),reply);
 i2c_init(i2c0,100000);gpio_set_function(SDA,GPIO_FUNC_I2C);gpio_set_function(SCL,GPIO_FUNC_I2C);
 // External 3.3 V pullups on carrier. Never rely on weak internal pulls.
 i2c_slave_init(i2c0,ADDRESS,i2cEvent);
 watchdog_enable(100,true); // MCU reset backup, NOT independent electrical cutoff
 while(true){
  const uint32_t now=to_ms_since_boot(get_absolute_time());
  uint8_t packet[SIZE];bool ready=false,fault=false;
  const uint32_t irq=save_and_disable_interrupts();
  ready=pendingReady;fault=busFault;if(ready)memcpy(packet,pending,SIZE);pendingReady=false;busFault=false;
  restore_interrupts(irq);
  const Inputs in=readInputs();
  if(fault)core.lock(9);
  else if(ready){Command c{};if(!decode(packet,SIZE,c))core.lock(1);else core.receive(c,now,in);}
  const auto d=core.tick(now,in);
  if(!d.permit){off();planner.reset();}
  else{
   CarrierOutputWords words{};
   if(!planner.convert({d.left,d.right,d.coil},now,true,words)){core.lock(10);off();}
   else{
    // PWM low before changing DIR; planner enforces measured deadtime.
    if(!words.pwmLeft)motorDuty(PWM_L,0);
    if(!words.pwmRight)motorDuty(PWM_R,0);
    gpio_put(DIR_L,words.dirLeft);gpio_put(DIR_R,words.dirRight);
    for(unsigned i=0;i<4;++i)gpio_put(COIL0+i,(words.solenoidMask&(1U<<i))!=0);
    motorDuty(PWM_L,words.pwmLeft);motorDuty(PWM_R,words.pwmRight);
    gpio_put(PERMIT,true);
   }
  }
  // WDI falling edge only after a fresh validated receive and completed loop.
  // Never derived from autonomous PWM/timer/DMA.
  if(d.pet&&(!d.permit||core.status(sample,in).state!=FAULT)){gpio_put(WDI,true);busy_wait_us_32(5);gpio_put(WDI,false);}
  uint8_t next[SIZE];encodeStatus(core.status(++sample,readInputs()),next);
  const uint32_t save=save_and_disable_interrupts();memcpy(reply,next,SIZE);restore_interrupts(save);
  watchdog_update();sleep_us(500);
 }
}
