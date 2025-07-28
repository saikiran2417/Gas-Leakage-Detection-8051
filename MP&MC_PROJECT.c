#include <reg51.h>
#include <intrins.h>

// Function prototypes
void i2c_start(void);
void i2c_stop(void);
void i2c_ACK(void);
void i2c_write(unsigned char);
void lcd_send_cmd(unsigned char);
void lcd_send_data(unsigned char);
void lcd_send_str(unsigned char *);
void lcd_slave(unsigned char);
void delay_ms(unsigned int);
void lcd_init(void);

// Define Gas sensor and LED pins
sbit gas_sensor = P1^0;  
sbit led = P0^6;         

// I2C Pins for LCD
sbit scl = P0^7;
sbit sda = P0^5;

// Slave address for I2C LCD
unsigned char slave1 = 0x4E;
unsigned char slave_add;

// I2C Functions
void i2c_start(void) {
    sda = 1; _nop_(); _nop_();
    scl = 1; _nop_(); _nop_();
    sda = 0; _nop_(); _nop_();
}

void i2c_stop(void) {
    scl = 0;
    sda = 0;
    scl = 1;
    sda = 1;
}

void i2c_ACK(void) {
    scl = 0;
    sda = 1;
    scl = 1;
    while (sda);
}

void i2c_write(unsigned char dat) {
    unsigned char i;
    for (i = 0; i < 8; i++) {
        scl = 0;
        sda = (dat & 0x80) ? 1 : 0;
        dat <<= 1;
        scl = 1;
    }
    scl = 0;
}

// LCD Functions
void lcd_slave(unsigned char slave) {
    slave_add = slave;
}

void lcd_send_cmd(unsigned char cmd) {
    unsigned char cmd_l, cmd_u;
    cmd_l = (cmd << 4) & 0xF0;
    cmd_u = (cmd & 0xF0);
    i2c_start();
    i2c_write(slave_add);
    i2c_ACK();
    i2c_write(cmd_u | 0x0C);
    i2c_ACK();
    delay_ms(1);
    i2c_write(cmd_u | 0x08);
    i2c_ACK();
    delay_ms(10);
    i2c_write(cmd_l | 0x0C);
    i2c_ACK();
    delay_ms(1);
    i2c_write(cmd_l | 0x08);
    i2c_ACK();
    delay_ms(10);
    i2c_stop();
}

void lcd_send_data(unsigned char dataw) {
    unsigned char dataw_l, dataw_u;
    dataw_l = (dataw << 4) & 0xF0;
    dataw_u = (dataw & 0xF0);
    i2c_start();
    i2c_write(slave_add);
    i2c_ACK();
    i2c_write(dataw_u | 0x0D);
    i2c_ACK();
    delay_ms(1);
    i2c_write(dataw_u | 0x09);
    i2c_ACK();
    delay_ms(10);
    i2c_write(dataw_l | 0x0D);
    i2c_ACK();
    delay_ms(1);
    i2c_write(dataw_l | 0x09);
    i2c_ACK();
    delay_ms(10);
    i2c_stop();
}

void lcd_send_str(unsigned char *p) {
    while (*p != '\0') {
        lcd_send_data(*p++);
    }
}

void delay_ms(unsigned int n) {
    unsigned int m;
    for (; n > 0; n--) {
        for (m = 121; m > 0; m--);
        _nop_();
    }
}

void lcd_init(void) {
    lcd_send_cmd(0x02);  // Return home
    lcd_send_cmd(0x28);  // 4-bit mode
    lcd_send_cmd(0x0C);  // Display ON, cursor OFF
    lcd_send_cmd(0x06);  // Increment cursor
    lcd_send_cmd(0x01);  // Clear display
}

void main(void) {
    unsigned char msg1[] = "**GAS DETECTED**";
    unsigned char msg2[] = "SAFE ENVIRONMENT";

    lcd_slave(slave1);
    lcd_init();

    while (1) {
        lcd_send_cmd(0x80);  // Set cursor to the first line

        if (gas_sensor == 0) {  // Gas detected
            int i;
            for (i = 0; i < 5; i++) {
                led = 1;
                delay_ms(200);
                led = 0;
                delay_ms(200);
            }
            lcd_send_str(msg1);  // Display "GAS DETECTED"
        } else {  // Safe environment
            led = 0;
            lcd_send_cmd(0x80);  // Set cursor to the second line
            lcd_send_str(msg2);  // Display "SAFE ENVIRONMENT"
        }
    }
}