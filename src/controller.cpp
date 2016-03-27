#include "controller.h"

Controller::Controller(QObject *parent) : QThread (parent)
{
	packetizer = new Packetizer("/dev/ttyUSB0", 0x0A);
	rpm_factor = 256.0/15000000.0/16.0;
	rpm_factor = 1.0/rpm_factor;
    emit ControllerFeedback(0, 0);

    start();
}

Controller::~Controller()
{

}

void Controller::run()
{
	uint8_t buf[256];
	int size;
    double current_rpm;
    double pwm_value;

    while(1)
    {
        //get rpm from microcontroller
        size = packetizer->get(buf);
        current_rpm = (buf[1] << 8 | buf[2]);
        current_rpm = current_rpm * rpm_factor;

        emit ControllerFeedback(0, current_rpm);

        //update parameters
        //pid.set_goal(current_goal);

        pwm_value = pid.update(current_rpm);

        if(pwm_value > 100)
            pwm_value = 100;
        else if(pwm_value < 0)
            pwm_value = 0;

        //send pwm value to microcontroller
        buf[0] = 0;
        buf[1] = pwm_value;
        packetizer->send(buf, 2);

        //sleep?
        //might not need to, packetizer.get() blocks for new data
    }

}
