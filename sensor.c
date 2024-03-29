#include "i2c-dev.h"
#include "ITG3205.h"
#include "ADXL345.h"
#include "HMC5883L.h"
//#include "gyro.h"
int file;

void  readBlock(uint8_t command, uint8_t size, uint8_t *data)
{
    int result = i2c_smbus_read_i2c_block_data(file, command, size, data);
    if (result != size)
    {
        printf("Failed to read block from I2C.");
        exit(1);
    }
}

void selectDevice(int file, int addr)
{
        char device[3];
        if (addr == 1)
                device == "L3G";
        else
                device == "LSM";


        if (ioctl(file, I2C_SLAVE, addr) < 0) {
                fprintf(stderr,
                        "Error: Could not select device  0x%02x: %s\n",
                        device, strerror(errno));
        }
}


void readACC(int  *a)
{
 	uint8_t block[6];
        selectDevice(file,ACC_ADDRESS);
		readBlock(0x80 | LSM303_OUT_X_L_A, sizeof(block), block);

        *a = (int16_t)(block[0] | block[1] << 8) >> 4;
        *(a+1) = (int16_t)(block[2] | block[3] << 8) >> 4;
        *(a+2) = (int16_t)(block[4] | block[5] << 8) >> 4;
}

void readMAG(int  *m)
{
	uint8_t block[6];
        selectDevice(file,MAG_ADDRESS);
	// DLHC: register address order is X,Z,Y with high bytes first
	readBlock(0x80 | LSM303_OUT_X_H_M, sizeof(block), block);

       	*m = (int16_t)(block[1] | block[0] << 8);
        *(m+1) = (int16_t)(block[5] | block[4] << 8) ;
        *(m+2) = (int16_t)(block[3] | block[2] << 8) ;
}
void readGYR(int *g)
{
	uint8_t block[6];

        selectDevice(file,GYR_ADDRESS);

	readBlock(0x80 | GYRO_XOUT_H, sizeof(block), block);

        *g = (int16_t)(block[0] << 8 | block[1]);
        *(g+1) = (int16_t)(block[2] << 8 | block[3]);
        *(g+2) = (int16_t)(block[4] << 8 | block[5]);

}


void writeAccReg(uint8_t reg, uint8_t value)
{
    selectDevice(file,ACC_ADDRESS);
  int result = i2c_smbus_write_byte_data(file, reg, value);
    if (result == -1)
    {
        printf ("Failed to write byte to I2C Acc.");
        exit(1);
    }
}

void writeMagReg(uint8_t reg, uint8_t value)
{
    selectDevice(file,MAG_ADDRESS);
  int result = i2c_smbus_write_byte_data(file, reg, value);
    if (result == -1)
    {
        printf("Failed to write byte to I2C Mag.");
        exit(1);
    }
}


void writeGyrReg(uint8_t reg, uint8_t value)
{
    selectDevice(file,GYR_ADDRESS);
  int result = i2c_smbus_write_byte_data(file, reg, value);
    if (result == -1)
    {
        printf("Failed to write byte to I2C Gyr.");
        exit(1);
    }
}


void enableIMU()
{

	__u16 block[I2C_SMBUS_BLOCK_MAX];

        int res, bus,  size;


        char filename[20];
        sprintf(filename, "/dev/i2c-%d", 1);
        file = open(filename, O_RDWR);
        if (file<0) {
		printf("Unable to open I2C bus!");
                exit(1);
        }

 // Enable accelerometer.        
        writeAccReg(ADXL345_POWER_CTL, 0b00000001); //  z,y,x axis enabled, 4Hz in sleep mode (not activated)
        writeAccReg(ADXL345_POWER_CTL, 0b00001001);
        writeAccReg(ADXL345_DATA_FORMAT, 0b00001110); // +/- 8G full scale: full resolution mode

 // Enable magnetometer
        writeMagReg(LSM303_MR_REG_M, 0x00);  // enable magnometer

 // Enable Gyro
       // writeGyrReg(L3G_CTRL_REG1, 0b00001111); // Normal power mode, all axes enabled
       // writeGyrReg(L3G_CTRL_REG4, 0b00110000); // Continuos update, 2000 dps full scale

	writeGyrReg(SMPLRT_DIV, 0b00000000); // Fsample = Finternal / (SMPLRT_DIV +1), where Finternal is either 1kHz or 8kHz
	writeGyrReg(DLPF_CFG, 0b00000000); // FS_SEL (full scale range), DLPF_CFG (low pass filter configuration and sample rate, 8 or 1 khz)
	writeGyrReg(INT_CFG, 0b00000000); // interrupt configuration
	writeGyrReg(INT_STATUS, 0b00000000); // interrupts status (bit2:PLL ready, bit0: Raw data is ready)
	writeGyrReg(PWR_MGM, 0b00000000); // reset device, sleep mode, stand by modes and clk_select

}



