#include <math/dl_math.hpp>

#pragma once

#ifndef MOTOR_H
#define MOTOR_H

struct Motor
{
    enum class Motor_dir{
        FORWARD,
        BACK,
        STOP
    };
    uint8_t Pin1;
    uint8_t Pin2;
    uint8_t PinEnb;
    Motor_dir Dir = Motor_dir::STOP;
    uint PowerCorrect = 200;
    uint Power = 0; 
    bool Revers = false;
    static uint MaxPower;
    Motor_dir Update();
    Motor_dir Update(Motor_dir);
};


Motor::Motor_dir Motor::Update(){
    return this->Update(this->Dir);
}

Motor::Motor_dir Motor::Update(Motor_dir dir){
    switch (dir)
    {
        case Motor_dir::BACK:{
            digitalWrite(this->Pin1, this->Revers ? LOW : HIGH);
            digitalWrite(this->Pin2, this->Revers ? HIGH : LOW);
            analogWrite(this->PinEnb, std::clamp((uint)this->Power, (uint)0, (uint)this->PowerCorrect));
            break;
        }
        case Motor_dir::FORWARD:{
            
            digitalWrite(this->Pin1, this->Revers ? HIGH : LOW);
            digitalWrite(this->Pin2, this->Revers ? LOW : HIGH);
            analogWrite(this->PinEnb, std::clamp((uint)this->Power, (uint)0, (uint)this->PowerCorrect));
            break;
        }
        case Motor_dir::STOP:{
            digitalWrite(Pin1, LOW);
            digitalWrite(Pin2, LOW);
            analogWrite(this->PinEnb, 0);
            break;
        }
    }
    this->Dir = dir;
    return dir;
}

#endif