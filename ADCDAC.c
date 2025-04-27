#include <ADUC841.H> 

unsigned int counter = 0 ;
unsigned int adc1_value, adc0_value;  
unsigned int dac_output_value; // DAC'a yazilan deger  



// DAC ayarlarini yapilandirma  
void DAC_Init(void) {  
        DACCON = 0x7F; // DAC ayarlari  
}  

// ADC ayarlarini yapilandirma  
void ADC_Init(void) {  
				ADCCON1 = 0xBC; // ADC mod ayarlari  
				ADCCON2 = 0x00; // ADC0 seçimi  
		}  

// Timer ayar fonksiyonu   
void Timer_Init() {  
				// Timer ayarlari  /0.1ms için timer
				TMOD |= 0x01; // Timer 0, mod 1 (16bit)  
				TH0 = 0xFB; // Timer baslangiç degeri (ayarlayin)   
				TL0 = 0xAC; // Timer baslangiç degeri (örtüsme olmasin)  
				TR0 = 1; // Timer 0'i baslat  
				ET0 = 1; // Timer 0 kesmesini etkinlestir  
				EA = 1; // Genel kesmeleri etkinlestir  
		}  

// UART'i baslat  
void UART_Init() {  
				SM0=0;
				SM1=1;				//veya SCON|=x50;
				TI=0;
				REN=1;
				TMOD |= 0x20;  // Timer 1, mod 2, otomatik yeniden yüklemeli  
				TH1 = 0xDC;    // Baud rate 9600 için (11.0592 MHz osilatör ile) 0XFD MI?
//			ET1 = 1 ;			 // Timer 1 kesmesini etkinlestir
				TR1 = 1;       // Timer 1'i baslat  
		}  

// UART ile veri gönderme  
void UART_Send(unsigned char data1) { 
	
				SBUF = data1;         // Veriyi gönder  
				while (TI==0){};   
				TI = 0;                // Gönderim bayragini sifirla  
		}  

		
		
		// ADC'den deger  kanal seçerek okuma 
unsigned int Read_ADC(unsigned char channel) 
{
    ADCCON2 = (ADCCON2 & 0xF0) | (channel & 0x0F);  // Üst 4bit sabit, alt 4bit kanal seçimi
    SCONV = 1; 
    while (SCONV == 1); 
    return ((unsigned int)(ADCDATAH & 0X0F) << 8) | (unsigned int)ADCDATAL; 
}

////// ADC0'dan deger okuma  
////unsigned int Read_ADC0(void) {
////  
////				ADCCON2 = 0x00; // ADC0'i seçer  
////				SCONV=1; // Dönüsümü baslat  
////				while (SCONV){}; // Dönüsüm tamamlanana kadar bekle  
////				return (ADCDATAL | ((ADCDATAH & 0x0F)<< 8)); // ADC verisini düzenle ve döndür   
////		}  

////// ADC1'den deger okuma  
////unsigned int Read_ADC1(void) { 
////	
////				ADCCON2 = 0x01; // ADC1'i seçer  
////				SCONV=1; // Dönüsümü baslat   
////				while (SCONV){}; // Dönüsüm tamamlanana kadar bekle  
////				return (ADCDATAL | ((ADCDATAH & 0x0F)<< 8)); // ADC verisini düzenle ve döndür    
////		}  

// DAC'ye veri yazma  
void Write_DAC0(unsigned int value) {
	
				if(value>4095){
				value=4095;}
				
				if(value<0){
				value=0;}
				 
				DAC0H = (value >> 8);  // Üst 8bit  
				DAC0L = value; // Alt 8bit DAC0'a yazilir  
				
}  

void Write_DAC1 (unsigned int value) {
	
				if(value>4095){
				value=4095;}
				
				if(value<0){
				value=0;}
				 
				DAC1H = (value >> 8);  // Üst 8bit  
				DAC1L = value; // Alt 8bit DAC0'a yazilir  
			}

void main(void) {  
				// Sadece LED kontrolü için Port 2 ve Port 3'ü temizle  
				P2 &= ~(1 << 3); // Port 2.3'ü temizle  
				P0 = 0x00;
				DAC_Init(); // DAC ayarlarini baslat  
				ADC_Init(); // ADC'yi baslat  
				Timer_Init(); // Timer'i baslat  
				UART_Init(); // UART'i baslat  

				while (1) 
					{
						
					}
				 
}  

// Timer kesme islevi  
void Timer_ISR(void) interrupt 1 {    
				 TF0 = 0;          // Timer 0 tasma bayragini temizleme 
				 counter++;
				 TH0 = 0xFB;       // Timer baslangiç degeri 
				 TL0 = 0xAC;       // Timer baslangiç degeri 
	
	 if (counter==100)       // 0.1ms*1000 =100 ms
	 { 
		 		counter=0;
        P0 = ~P0; 				// LED'i tersle   

		 
		    adc0_value = Read_ADC(0); // ADC0'dan deger oku  
				adc1_value = Read_ADC(1); // ADC1'den deger oku  

// ADC çikislarinin farkini hesapla ve DAC'ye yaz  
//     dac_output_value = 2 * (adc0_value - adc1_value);  
        Write_DAC0(adc0_value); // DAC'ye yaz  
				Write_DAC1(adc1_value);
 

// Yazilan DAC çiktisini UART ile gönder 
		 	UART_Send('s');        //Baslangiç için 
		 
		 	UART_Send(adc0_value);        // Düsük byte'i gönder 
			UART_Send((adc0_value >> 8)); // Yüksek byte'i gönder   
      UART_Send(adc1_value);        // Düsük byte'i gönder 
			UART_Send((adc1_value >> 8)); // Yüksek byte'i gönder 
		 		 		
			UART_Send('e');        // sonlandirma için
//		 
	

			}

	}
    

 