#include <MAX72xx.h>

MAX72xx *led;
int phase = 0;
char message[64];
int messageLength = 0;
int myUptime = 0;

uint8_t data = D0;
uint8_t load = D1;
uint8_t myclock = D2;

void setup()
{
    led = new MAX72xx(data,myclock,load,1); //数据引脚,时钟引脚,片选引脚,控制的设备数
    led-> shutdown(0,false); //正常启动
    led-> setIntensity(0,1); //设置亮度 0-15级
}

void loop()
{

    if(phase==0) //Message-loop 开始
    {
        myUptime = (int)(millis()/1000);
        sprintf(message,"Uptime %d:%02d", myUptime / 3600, (myUptime % 3600)/60); //更新消息
        messageLength = strlen(message); // 计算数量
        led->tweenLetters(0,' ',message[phase]); //从空开始到最后一个字开始滚动
    }

    if(phase<messageLength-1)//下一个字符除了最后一个
    {
        led->tweenLetters(0,message[phase],message[phase+1]);
        phase++;
    }
    else if(phase==messageLength-1) //最后一个字后就滚动到空
    {
        led->tweenLetters(0,message[phase],' ');
        phase = 0; //restart message-loop
    }
}
