 #include "xieyi.h"
 //1.引用外部函数 
 void SET_SLEEPTIME(time_t t);
 //设置亮屏时间和息屏到休眠时间
void SET_Last_span_Sleep_span(int x,int y);
//2.填写指令参数格式
  NAME_TAB name_tab[]=
 {
	  (void*)SET_SLEEPTIME,                                   {0xaa,0x00, 1, 1, 1, 1, 0, 0,0,0, 0xbb},
      (void*)SET_Last_span_Sleep_span,                        {0xaa,0x01, 1, 1, 1, 1, 2, 2,2,2, 0xbb},
 };
//3.根据函数表修改xieyi.h的tab
//4.在主程序中调用xieyi_scan();



 
  void init_xieyi(uint8_t *p)
 {
     *p = (sizeof(name_tab)/sizeof(name_tab[0]));
 } 
 
NAME_par  M_name_par =
{
	 name_tab,
	 exe,
	 0,//ID
	 0,//CMD
	 0,// uint8_t GS;
	 0,//uint8_t pra1;
	 0,//uint8_t cs2;
};


