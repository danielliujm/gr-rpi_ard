const int motorPins[8] = {2, 3, 4, 5, 6, 7, 8, 9};
double PH_1 = 1.0;
double PH_2 = 2.0;
double ORP_1 = 3.0;
double ORP_2 = 4.0;

uint8_t PH1_ADDR = 0x65;
uint8_t PH2_ADDR = 0x67;
uint8_t ORP1_ADDR = 0x66;
uint8_t ORP2_ADDR = 0x68;

int FLOAT_SWITCH_PIN_1 = 10;
int FLOAT_SWITCH_PIN_2 = 11;

double EFFLUENT_DUTY_1 = 0.0 ;
double EFFLUENT_DUTY_2 = 0.0 ;
double EFFLUENT_DUTY_3 = 0.0 ;
double EFFLUENT_DUTY_4 = 0.0 ;
double EFFLUENT_DUTY_5 = 0.0 ;
double EFFLUENT_DUTY_6 = 0.0 ;

double INLET_DUTY_1 = 0.0 ;
double INLET_DUTY_2 = 0.0 ;
double KI_EFFLUENT_1 = 0.0;
double KI_EFFLUENT_2 = 0.0;
double KI_INLET_1 = 2.0;
double KI_INLET_2 = 2.0;

double currentPWMValues[8] = {0,0,0,0,0,0,0,0};

int prev_time; 
int SAVE_INTERVAL = 10000;


const int TX_PIN_1 = 30;
const int TX_PIN_3 = 45;
const int TX_PIN_5 = 43;
const int TX_PIN_2 = 49;
const int TX_PIN_4 = 51;
const int TX_PIN_6 = 48; 


const int RX_PIN_1 = 52;
const int RX_PIN_2 = 52;
const int RX_PIN_3 = A10;
const int RX_PIN_4 = 50;
const int RX_PIN_5 = A10;
const int RX_PIN_6 = 53;








const int32_t RUN_VELOCITY = 40000;

TMC2209 stepper_drivers[6] = {};
TMC2209 stepper_driver_1, stepper_driver_2, stepper_driver_3, stepper_driver_4, stepper_driver_5, stepper_driver_6;

SoftwareSerial port1 (RX_PIN_1, TX_PIN_1);
SoftwareSerial port3(RX_PIN_3, TX_PIN_3);
SoftwareSerial port5(RX_PIN_5, TX_PIN_5);

SoftwareSerial port2 (RX_PIN_2, TX_PIN_2);
SoftwareSerial port4(RX_PIN_4, TX_PIN_4);
SoftwareSerial port6(RX_PIN_6, TX_PIN_6);



const int RUN_CURRENT_PERCENT = 100; // Set the run current to 100% of the maximum


const int SPEED = 100;