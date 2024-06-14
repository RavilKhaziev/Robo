#pragma once

namespace RoboDrive{

    template <typename StateEnum>
    class IState{
        protected: 
            StateEnum _state;
        public:
        virtual StateEnum GetState() const{
            return this->_state;
        }
        virtual void Start();
        virtual void Update();
        virtual void NextState();
        virtual void ReturnToLast(); 
    };

}