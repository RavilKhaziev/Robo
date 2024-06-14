#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include "IState.h"
#include "IStateMachine.h"

#include "iostream"

namespace RoboDrive{
    enum class RoboState {
        WAIT_TO_CONFIG, // Ожидание настройки устройства указание SSID и пароля для передачи
        CONNECT_TO_SERVER, // Попытка соединения с сервером 
        REMOTE_SERVER, // Соединение прошло успешно. Слушаем команды сервера.
        ERROR_ROBO // Произошла внутреняя ошибка
    };

    class RoboStateMachine : IStateMachine<RoboState>{
        private: 
            std::vector<IState<RoboState>>::iterator _curentState;
            std::vector<IState<RoboState>> _states;
        public:
            RoboStateMachine(){}
            ~RoboStateMachine(){}

            virtual void AddState(IState<RoboState> *state){
                
                std::cout << "Try add state to StateMachine:" << (int)(*state).GetState() << std::endl;
                auto iter = std::find_if_not(_states.begin(), _states.end(), 
                [&state](IState<RoboState> x) -> bool
                {
                    return x.GetState() == (*state).GetState();
                });
                if(iter != _states.end()){
                    
                    std::cout << "Add state to StateMachine" << (int)(*state).GetState() << "fail";
                    return;
                }
                _states.push_back(std::move(*state));
            }

            virtual bool Start(const RoboState &state){
                
                if(_states.size() == 0){
                    std::cout << "Кол-во статусов = 0" << std::endl;
                    return false;
                }
                NextState(state);
                return true;
            }

            virtual void NextState(const RoboState &state){
                auto iter = std::find_if(_states.begin(), _states.end(), [state](RoboDrive::IState<RoboState> &x){return x.GetState() == state;});
                if(iter != _states.end()){
                    _curentState = _states.begin();
                    return;
                }
                _curentState = iter;
            }
    };

}

