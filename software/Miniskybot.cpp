//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//-- Miniskybot library
//------------------------------------------------------------------------------
//-- Simplify working and programming a Miniskybot (or similar) robot by using
//-- this library.
//--
//-- It contains functions to control a pair of motors and some sensors. 
//--
//-- For more info, read README
//------------------------------------------------------------------------------
//-- Author:
//-- David Estévez-Fernández, Jun 2012
//-- GPL license
//------------------------------------------------------------------------------
//-- Requires a Miniskybot  robot, design by Juan González-Gómez (Obijuan):
//-- https://github.com/Obijuan/Miniskybot
//------------------------------------------------------------------------------

#include "Miniskybot.h"

//-- Constructor
Miniskybot::Miniskybot()
{
	_num_motors = 0;
    _num_servos = 0;
	_num_US_sensor = 0;
	_num_IR_sensor = 0;
	
	_velocity = 0;
	_angularVelocity = 0;
}

	
//-- Setup:
//-----------------------------------------------------
//-- Add elements
void Miniskybot::addMotor( int pinLeft, int pinRight, int pinEnable )
{
	if ( _num_motors < MAX_MOTORS)
	{
		motor[_num_motors].attach( pinLeft, pinRight, pinEnable);
		_num_motors++;
	}
}
void Miniskybot::addServo( int pin, pos position )
{
    if ( _num_servos < MAX_MOTORS)
    {
        servo[_num_servos].attach( pin, position);
        _num_servos++;
    }
}

void Miniskybot::addSensor( int type, int pin)
{
	if ( type == IR )
	{
		if ( _num_IR_sensor < MAX_SENSORS_IR )
		{
			sensor_IR[_num_IR_sensor].attach( pin, CALIBRATION_PARAM[_num_IR_sensor][0],CALIBRATION_PARAM[_num_IR_sensor][1] );
			_num_IR_sensor++;
		}
	
	}
	else if (type == US_3PIN)
	{
		if ( _num_US_sensor < MAX_SENSORS_US )
		{
			sensor_US[_num_US_sensor].attach( pin);
			_num_US_sensor++;
		}
	}

}


void Miniskybot::addSensor( int type, int pinTrigger, int pinEcho)
{
	if (type == US_4PIN)
	{
		if ( _num_US_sensor < MAX_SENSORS_US )
		{
			sensor_US[_num_US_sensor].attach( pinTrigger, pinEcho);
			_num_US_sensor++;
		}
	}
}

void Miniskybot::addSensor( int type, int pin, pos position)
{
    if (type == FOLLOW){
        if (_num_FOLLOW_sensor < MAX_SENSORS_FOLLOW){
            sensor_FOLLOW[_num_FOLLOW_sensor].attach( pin,position);
            _num_FOLLOW_sensor++;
        }
    }
    else if (type == LIGHT){
        if (_num_LIGHT_sensor < MAX_SENSORS_LIGHT){
            sensor_LIGHT[_num_LIGHT_sensor].attach( pin,position);
            _num_LIGHT_sensor++;
        }
    }

}

//-- Movement control:
//-----------------------------------------------------
//-- Access individual elements (or all elements if index == -1)

//-- Gives a motor the control value [0-255] 
void Miniskybot::motorControl( short value , int index)
{
	if (index == -1 )
	{
		//-- Set velocity in all motors
		for (int i = 0; i < _num_motors; i++)
		{
			motor[i].setVelocity( value);
		}
	}
	else
	{	
		//-- Set velocity in just one motor
		if (index < _num_motors)
			motor[index].setVelocity( value );
	}
}
//-- Gives a servo the control value [0-255]
void Miniskybot::servoControl( signed int value , int index)
{
    if (index == -1 )
    {
        //-- Set velocity in all motors
        for (int i = 0; i < _num_motors; i++)
        {
            servo[i].setVelocity( value);
        }
    }
    else
    {
        //-- Set velocity in just one motor
        if (index < _num_servos)
            servo[index].setVelocity( value );
    }
}
//-- Sets a motor with the velocity suggested
void Miniskybot::motorVelocity( int velocity, int index)
{
	//-- Looks for the control value in the table:
	short value = lookUp(velocity);

	//-- Sets that value
    motorControl(value , index);
}

//-- Robot control
void Miniskybot::move( float velocity, float angularVelocity)
{
	//-- This function gives priority to turning over linear speed
    float v_left, v_right;
	//-- This action can only be executed with two wheels
	if( _num_motors == MAX_MOTORS )
	{
		//-- Calculate max angular velocity
		float omega_max = 2*VELOCITY_TABLE[0][0]/DIST_WHEEL;
		
		//-- If angular velocity requested is larger than max, use max angular speed
		if ( abs(angularVelocity) > omega_max) { angularVelocity > 0 ? angularVelocity = omega_max : angularVelocity = -omega_max;}

		//-- Find max linear velocity, given that angular velocity
		float linear_max = VELOCITY_TABLE[0][0] - ( DIST_WHEEL / 2.0) * abs(angularVelocity);

		//-- If angular velocity requested is larger than max, use max angular speed
		if ( abs(velocity) > linear_max) { velocity > 0 ? velocity = linear_max : velocity = -linear_max;}

		//-- Calculate the needed speed of each wheel:

		v_right = velocity + angularVelocity * DIST_WHEEL / 2.0;
		v_left = velocity - angularVelocity * DIST_WHEEL / 2.0;

		//-- Look for the values corresponging to that speeds:
		short value_left, value_right;
		value_left = v_left/abs(v_left)*lookUp( abs(v_left));
		value_right = v_right/abs(v_right)*lookUp( abs(v_right));

		//-- Set the speed to the motors:
		motor[0].setVelocity( value_left);
		motor[1].setVelocity( -value_right);
	}
    else if ( _num_servos == MAX_MOTORS)
        v_right = (velocity - angularVelocity)/ 2.0;
        v_left = (velocity + angularVelocity)/ 2.0;
        if(servo[0].getPosition() == LEFT){

            servo[0].setVelocity(v_left);
        }
        else{
            servo[0].setVelocity(v_right);
        }
        if(servo[1].getPosition() == LEFT){

            servo[1].setVelocity(v_left);
        }
        else{
            servo[1].setVelocity(v_right);
        }
}

//-- Sensor data:
//----------------------------------------------------
float Miniskybot::getDistance( int type, int sensor )
{
	if ( type == IR )
	{
		if ( sensor < _num_IR_sensor )
			return sensor_IR[sensor].getLength();
	}
	else if (type == US || type == US_3PIN || type == US_4PIN)
	{
		if (sensor < _num_US_sensor)
			return sensor_US[sensor].getLength();
	}
}

//-- Look for the value corresponding to a certain velocity in the table
short Miniskybot::lookUp( float target)
{
	//-- Search index
	int lower, middle, top;
	lower = 0;
	top = NUM_VALUES-1;
	
	//--Current value
	float value;
	
	while (lower <= top)
	{
		middle = (top+lower)/2;
		value = VELOCITY_TABLE[middle][0];
		
		if ( value == target )
			return VELOCITY_TABLE[middle][1];
		else if (value < target )
			top = middle-1;
		else if (value > target )
			lower = middle+1;
	}
	
	return VELOCITY_TABLE[middle][1];
}

//----------------------------------------------------
void Miniskybot::followLine()
{
    int state = 0;
    // 0 = Search for a line
    // 1 = Move forward
    // 2 = Move right
    // 3 = Move left
    bool s_left,s_right;
    //-- Read the value of every sensor.
   for (int i =0;i<MAX_SENSORS_FOLLOW;i++){
    if( sensor_FOLLOW[i].getPosition() == LEFT )
        s_left = sensor_FOLLOW[i].getValue();
    else{
        s_right = sensor_FOLLOW[i].getValue();
    }
   }
    //-- Execute the control of the robot
    if (s_left == BLACK && s_right == BLACK){  //-- Move to forward
         state = 1;}
    else if (s_left == WHITE && s_right == BLACK){ //-- Move to right
         state = 2;}
    else if (s_left == BLACK && s_right == WHITE){ //-- Move to left
         state = 3;}
    else if (s_left == WHITE && s_right == WHITE){ //-- Search a line
          state = 0;
    }
    switch (state){
        case 0:
            move(1,2);
            break;
        case 1:
            move(10,0);
            break;
        case 2:
            move(10,10);
            break;
        case 3:
            move(10,-10);
            break;
        default :
            move(0,0);
            break;
    }
}
int Miniskybot::getLight(int sensor )
{

        if ( sensor < _num_LIGHT_sensor )
            return sensor_LIGHT[sensor].getValue();
        else
            return -1;

}



