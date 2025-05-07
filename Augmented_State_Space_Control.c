#include <ADUC841.H>
#include <math.h>

// K parametreleri ve degiskenler    
float  K1= 0.0632 ;
float  Ki=-0.12665;
float Kb = 0.75;

unsigned int x1 = 0, counter = 0, x2 = 0 , r=0;  
float kontrol_sinyali = 0 ,onceki_integraldegeri = 0.0 ,integral=0.0 ,onceki_hata=0.0 ;    
int hata=0;

// Prototipler  
void DAC_Init(void);
void ADC_Init(void);
void Timer_Init(void);
unsigned int Read_ADC(unsigned char channel);
void Write_DAC0(unsigned int value);

// Main fonksiyonu  
void main(void) {

    DAC_Init();       // DAC ayarlarini baslat  
    ADC_Init();       // ADC'yi baslat  
    Timer_Init();     // Timer'i baslat  
	
    while (1) {
		
		
							 }
		
							
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
    TMOD |= 0x01;  // Timer 0, mod 1 (16bit)  
    TH0 = 0xD4;    // Timer baslangiç degeri (Upper Byte)
    TL0 = 0xCD;    // Timer baslangiç degeri (Lower Byte)
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
		value &=0xFFF;
    DAC0H = (value >> 8)&0x0F;  // Üst 8bit  
    DAC0L = value & 0xFF;       // Alt 8bit DAC0'a yazilir  
}

// Timer kesme islevi  
void Timer_ISR(void) interrupt 1 {  
    
    counter++;
    TH0 = 0xD4;       // Timer baslangiç degeri  
    TL0 = 0xCD;       // Timer baslangiç degeri  

    if (counter == 12) {  // Örnekleme zamani ms cinsinden  Ts= 1*counter
        counter = 0;  
										
	
    x1 = Read_ADC(2); // ADC'den deger oku ölcülen 
		r  = Read_ADC(7); // ADC'den deger oku referans
		
   //KONTROLöR	
    hata = r - x1;
		integral=onceki_integraldegeri + onceki_hata; //antiwindup buraya yazilcak 
		kontrol_sinyali=((-K1*x1)+(-Ki*integral));
// Anti-Windup mekanizmasi - Geri Hesaplama (Back Calculation)
    if (kontrol_sinyali > 4095.0f) {
   
    integral = integral - (Kb * (kontrol_sinyali - 4095.0f));
		kontrol_sinyali=4095;
}
		else if (kontrol_sinyali < 0.0f) {
    
    integral = integral -( Kb * (kontrol_sinyali));  // dikkat: zaten negatif
		kontrol_sinyali=0;
}
    // kontrolör cikisini DAC’a yaz  
    Write_DAC0(kontrol_sinyali);
		onceki_integraldegeri = integral ;
		onceki_hata = hata;
		

		}
}
