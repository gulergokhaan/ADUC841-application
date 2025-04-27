#include <ADUC841.H>

// PID parametreleri ve degiskenler  
#define Kp 0.0462  
#define Ki 0.0816  
#define Kd -3.2501  

unsigned int istenen_deger = 0.0;  
unsigned int olculen_deger = 0.0;  
unsigned int kontrol_sinyali = 0.0;  
float hata = 0, up = 0, ui = 0, ud = 0;  
float integral = 0, turev = 0, oncekiintegraldegeri = 0;  

unsigned int counter = 0;

// Prototipler  
void DAC_Init(void);
void ADC_Init(void);
void Timer_Init(void);
//void UART_Init(void);
unsigned int Read_ADC(unsigned char channel);
void Write_DAC0(unsigned int value);
void PID_Control(void);
void Timer_ISR(void);

// Main fonksiyonu  
void main(void) {
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
    TH0 = 0xFB;    // Timer baslangiç degeri  
    TL0 = 0xAC;    // Timer baslangiç degeri  
    TR0 = 1;       // Timer 0'i baslat  
    ET0 = 1;       // Timer 0 kesmesini etkinlestir  
    EA = 1;        // Genel kesmeleri etkinlestir  
}

//// UART baslatma  
//void UART_Init(void) {  
//    SM0 = 0;
//    SM1 = 1;       // veya SCON |= x50  
//    TI = 0;
//    REN = 1;
//    TMOD |= 0x20;  // Timer 1, mod 2, otomatik yeniden yükleme  
//    TH1 = 0xDC;    // Baud rate 9600 için (11.0592 MHz osilatör ile)  
//    TR1 = 1;       // Timer 1'i baslat  
//}

// ADC0'dan deger okuma  
unsigned int Read_ADC(unsigned char channel) {
    ADCCON2 = (ADCCON2 & 0xF0) | (channel & 0x0F); // Kanal seçimi  
    SCONV = 1; 
    while (SCONV == 1) {};          
    return ((unsigned int)(ADCDATAH & 0X0F) << 8) | (unsigned int)ADCDATAL; 
}

// DAC'ye veri yazma  
void Write_DAC0(unsigned int value) {
    if (value > 4095) value = 4095;
    if (value < 0) value = 0;
    DAC0H = (value >> 8);  // Üst 8 bit  
    DAC0L = value;         // Alt 8 bit DAC0'a yazilir  
}

// PID kontrol fonksiyonu  
void PID_Control(void) {
    // ADC'den deger okuma  
    istenen_deger = Read_ADC(7); // ADC'den deger oku  
    olculen_deger = Read_ADC(2); // ADC'den deger oku  

    // Hata hesaplama  
    hata = istenen_deger - olculen_deger;

    // Oransal terim  
    up = (float)(Kp * hata);

    // Integral terimi  
    integral = oncekiintegraldegeri + hata;  // Integral degerini biriktir  
    ui = (float)(Ki * integral);
    oncekiintegraldegeri = integral;

    // Türev terimi  
    ud = (float)(Kd * (hata - turev));
    turev = hata;

    // PID çikisi  
    kontrol_sinyali = (float)(up + ui + ud);

    // Çikis sinirlandirma  
    if (kontrol_sinyali > 4095) kontrol_sinyali = 4095;
    if (kontrol_sinyali < 0) kontrol_sinyali = 0;

    // PID çikisini DAC’a yaz  
    Write_DAC0(kontrol_sinyali);
}

// Timer kesme islevi  
void Timer_ISR(void) interrupt 1 {  
    TF0 = 0;          // Timer tasma bayragini temizle  
    counter++;
 

    if (counter == 1150) {  // Örnekleme zamani: 0.1ms * 1150 = 115ms  
        counter = 0;  
//        P0 = ~P0;           // Ledleri tersleme islemi  
        PID_Control();      // PID kontrol fonksiyonunu çagir (yalnizca bu kesmede)  
			   TH0 = 0xFB;       // Timer baslangiç degeri  
         TL0 = 0xAC;       // Timer baslangiç degeri  
    }
}
