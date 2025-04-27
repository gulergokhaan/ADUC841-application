#include <ADUC841.H>
#include <math.h>

// PID parametreleri ve degiskenler  
float  Kp=0.0462  ;
float  Ki= 0.0816  ;
float  Kd =-3.2501;  
float  T=0.12;
float  Kb= (1/T);     // Geri hesaplama kazanci (ANTIWIND-UP)(Kb)


unsigned int istenen_deger = 0,counter = 0,olculen_deger = 0;  
float up = 0, ui = 0, ud = 0 ,kontrol_sinyali = 0 ;  
float integral = 0.0, turev = 0.0, oncekiintegraldegeri = 0.0;  
int hata=0;

// Prototipler  
void DAC_Init(void);
void ADC_Init(void);
void Timer_Init(void);
unsigned int Read_ADC(unsigned char channel);
void Write_DAC0(unsigned int value);

// Main fonksiyonu  
void main(void) {
//		Kb = sqrt(abs(Ki/Kd));
//    P2 &= ~(1 << 3);  // Port 2.3'ü temizle  
//    P0 = 0x00;        // Port sifirlama  
    DAC_Init();       // DAC ayarlarini baslat  
    ADC_Init();       // ADC'yi baslat  
    Timer_Init();     // Timer'i baslat  
    while (1) {}
}

// DAC ayarlarini yapilandirma  
void DAC_Init(void) {  
    DACCON = 0x6D; // DAC ayarlari  
}

// ADC ayarlarini yapilandirma  
void ADC_Init(void) {  
    ADCCON1 = 0xFC; // ADC mod ayarlari  
    ADCCON2 = 0x00; // ADC0 seçimi  
}

// Timer ayar fonksiyonu (0.1ms için timer)  
void Timer_Init(void) {  
    TMOD |= 0x01;  // Timer 0, mod 1 (16-bit)  
    TH0 = 0xD4;    // Timer baslangiç degeri  
    TL0 = 0xCD;    // Timer baslangiç degeri  
    TR0 = 1;       // Timer 0'i baslat  
    ET0 = 1;       // Timer 0 kesmesini etkinlestir  
    EA = 1;        // Genel kesmeleri etkinlestir  
}

// ADC0'dan deger okuma  
unsigned int Read_ADC(unsigned char channel) {
    ADCCON2 = (ADCCON2 & 0xF0) | (channel & 0x0F); // Kanal seçimi  
    SCONV = 1; 
    while (SCONV == 1) {};          
    return ((unsigned int)(ADCDATAH & 0X0F) << 8) | (unsigned int)ADCDATAL; 
}

// DAC'ye veri yazma  
void Write_DAC0(unsigned int value) {
//    if (value > 4095) value = 4095;
//    if (value < 0) value = 0;
		value &=0xFFF;
    DAC0H = (value >> 8)&0x0F;  // Üst 8bit  
    DAC0L = value & 0xFF;       // Alt 8bit DAC0'a yazilir  
}

// Timer kesme islevi  
void Timer_ISR(void) interrupt 1 {  
           // Timer tasma bayragini temizle  
    counter++;
    TH0 = 0xD4;       // Timer baslangiç degeri  
    TL0 = 0xCD;       // Timer baslangiç degeri  

    if (counter == 12) {  // Örnekleme zamani: 10ms 
        counter = 0;  
										
					
    istenen_deger = Read_ADC(7); // ADC'den deger oku  
    olculen_deger = Read_ADC(2); // ADC'den deger oku  

    // Hata hesaplama  	
    hata = istenen_deger - olculen_deger;

    // Oransal terim  
    up = (float)(Kp * hata);
    integral = oncekiintegraldegeri + hata;  // Integral degerini biriktir  
		// Integral terimi 
    ui = (float)(Ki * integral);    
    oncekiintegraldegeri = integral;	  // Integral birikimini güncelle  
		// Türev terimi 			
    ud = (float)(Kd * (hata - turev));  
    turev = hata;

    // PID çikisi  
    kontrol_sinyali = (float)(up + ui + ud);
		
   		// Anti-Windup mekanizmasi - Geri Hesaplama (Back Calculation)
    if (kontrol_sinyali > 4095) {
        integral = integral - (Kb*(kontrol_sinyali - 4095)) ;  // Üst saturasyon düzeltmesi
    } 
    else if (kontrol_sinyali < 0) {
        integral = integral - (Kb*(kontrol_sinyali)) ;    // Alt saturasyon düzeltmesi
			
		}

    // Çikis sinirlandirma  
		
//   if (kontrol_sinyali > 4095) {kontrol_sinyali = 4095;}
//   if (kontrol_sinyali < 0) {kontrol_sinyali = 0;}

    // PID çikisini DAC’a yaz  
    Write_DAC0(kontrol_sinyali);

		}
}
