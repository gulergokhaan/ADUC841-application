#include <ADuC841.h>
sbit BUTON2 = P2^4;

// Global degiskenler
unsigned int sayac = 0, ref = 0, ref_hold = 0, cikis = 0;
float up = 0, ui = 0, ud = 0, Upid = 0;
int hata = 0;

// PID sabitleri
float Kp = 0.0462;
float Ki = 0.0817;
float Kd = -3.6619;

// PID integral ve türev hesaplari için yardimci degiskenler
float turev = 0.0, integral = 0.0, A = 0.0;
bit buton_onceki_durum = 1; // Butonun önceki durumunu saklar

// Fonksiyon Prototipleri
void ADC_ayar(void);
void DAC_ayar(void);
void Timer0_ayar(void);
unsigned int Read_ADC(unsigned char channel);
void Write_DAC(unsigned int value);

void main() 
{
    ADC_ayar();    // ADC baslat
    DAC_ayar();    // DAC baslat
    Timer0_ayar(); // 1ms Timer baslat

    while (1) 
    {
        // Timer kesmesi islemleri gerçeklestirecektir.
    }
}

// ADC baslatma
void ADC_ayar(void) 
{
    ADCCON1 = 0xFC;  
    ADCCON2 = 0x00;  
}

// DAC baslatma
void DAC_ayar(void) 
{
    DACCON = 0x6D;  
}

// Timer0 baslatma (1ms kesme süresi)
void Timer0_ayar(void) 
{
    TMOD |= 0x01; 
    TH0 = 0xD4;   
    TL0 = 0xCD;
    ET0 = 1;  
    TR0 = 1;  
    EA = 1;   
    sayac = 0;
}

// ADC'den belirlenen kanaldan okuma yap
unsigned int Read_ADC(unsigned char channel) 
{
    ADCCON2 = (ADCCON2 & 0xF0) | (channel & 0x0F);  // Üst 4 bit sabit, alt 4 bit kanal seçimi
    SCONV = 1; 
    while (SCONV == 1); 

    return ((unsigned int)(ADCDATAH & 0X0F) << 8) | (unsigned int)ADCDATAL; 
}

// DAC0 çikisina veri yazma
void Write_DAC(unsigned int deger) 
{
    deger &= 0x0FFF;  
    DAC0H = (deger >> 8) & 0x0F;  
    DAC0L = deger & 0xFF;         
}

// Timer0 kesme rutini
void Timer0_ISR(void) interrupt 1 
{
    TH0 = 0xD4;  
    TL0 = 0xCD;
    sayac++;

    if (sayac == 12) // 10ms örnekleme zamani
    {
        sayac = 0;

        // Butona ilk kez basildiginda ref_hold kaydedilsin
        if (BUTON2 == 0 && buton_onceki_durum == 1) // Butona yeni basildi
        {
            ref_hold = Read_ADC(7); // O anki ref degerini sakla
        }

        // Buton basiliysa ref_hold kullan, degilse ADC'den oku
        if (BUTON2 == 0) 
				{ ref = ref_hold;} // Sabit tut
        
        else
        {ref = Read_ADC(7);}

        buton_onceki_durum = BUTON2; // Önceki buton durumunu sakla

        cikis = Read_ADC(2); // Çikis degerini oku

        // PID hesaplama
        hata = ref - cikis; 

        // Oransal
        up = (float)(Kp * hata);

        // Integral
        integral = A + hata;
        ui = (float)(Ki * integral);
        A = integral;

        // Türev
        ud = (float)(Kd * (hata - turev));
        turev = hata;

        // PID Çikisi
        Upid = (float)(up + ui + ud);
        
        if (Upid > 4095) Upid = 4095;
        if (Upid < 0) Upid = 0;

        Write_DAC(Upid);
    }
}
